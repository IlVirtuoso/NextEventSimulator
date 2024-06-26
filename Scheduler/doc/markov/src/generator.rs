use std::{collections::VecDeque, io::Cursor, vec};

use indicatif::ProgressBar;
use nalgebra::{DMatrix, SVector};


use crate::{
    parameters::Params,
    state::{GraphElement, State, StateDescriptor},
    transition::Transition,
};

struct NodeGenerator {
    generated: Vec<State>,
}

impl NodeGenerator {
    pub fn new() -> Self {
        return NodeGenerator {
            generated: Vec::new(),
        };
    }

    pub fn add_state(&mut self, state: &State) {
        if state.is_valid() {
            self.generated.push(state.clone());
        }
    }

    pub fn index_of(&self, state: &State) -> usize {
        let index = self.generated.iter().position(|x| x == state).unwrap();
        index
    }

    pub fn generate(&mut self) {
        let mut cursor = SVector::<u8, 4>::zeros();
        while cursor[0] != Params::instance().numclients {
            for i in (0..4).rev() {
                if cursor[i] < Params::instance().numclients {
                    cursor[i] += 1;
                    break;
                } else {
                    cursor[i] = 0;
                }
            }
            let mut state = State::new(cursor[0], cursor[1], cursor[2], cursor[3], 1);
            if !state.is_valid() {continue;}
            self.add_state(&state);
            if state.n_cpu > 0 {
                state.cpu_stage = 2;
                self.add_state(&state)
            }
        }
    }
}

#[derive(Clone)]
pub struct Graph{
    nodes: Vec<State>,
    edges: Vec<Transition>
}


impl Graph {

    pub fn new() -> Self{
        Graph { nodes: Vec::new(), edges: Vec::new() }

    }

    pub fn adj_matrix(&self) -> DMatrix<f64> {
        let nodes = &self.nodes;
        let mut mat = DMatrix::<f64>::zeros(nodes.len(), nodes.len());
        let mut progress = ProgressBar::new(mat.len() as u64).with_message("Generating matrix");
        for i in 0..nodes.len() {
            let reference = nodes[i];
            for j in 0..nodes.len() {
                let mut tr = self.get_transition(&nodes[i], &nodes[j]);
                mat[(i, j)] = if tr.is_none() {
                    0.0
                } else {
                    tr.unwrap().probability()
                };
                progress.inc(1);
            }
        }
        mat
    }   
    
    pub fn get_transition(&self, head: &State, tail: &State) -> Option<Transition> {
        for tr in &self.edges {
            if *tr.head() == *head && *tr.tail() == *tail {
                return Some(tr.clone());
            }
        }
        None
    }

    pub fn add_edge(&mut self, transition: Transition) {
        self.edges.push(transition);
    }

    pub fn to_flow_chart(&mut self) -> String {
        let mut lines: Vec<String> = Vec::<String>::new();
        lines.push("source,target,value".to_string());

        for tr in &mut self.edges {
            lines.push(format!(
                "{},{},{}",
                tr.head().to_graph_element(),
                tr.tail().to_graph_element(),
                tr.probability()
            ));
        }
        lines.join("\n")
    }

    pub fn to_dot(&mut self) -> String {
        let mut lines = Vec::<String>::new();
        lines.push("digraph {".to_string());
        lines.push("rankdir=TB".to_string());
        lines.push("overlap=scale".to_string());
        for edge in &mut self.edges {
            lines.push(format!(
                "\"{}\"->\"{}\"[label=\"{:.4}\"]",
                edge.head().to_graph_element(),
                edge.tail().to_graph_element(),
                edge.probability()
            ));
        }
        lines.push("}".to_string());
        lines.join("\n")
    }

    pub fn nodes(&self) -> &Vec<State>{
        &self.nodes
    }

    pub fn edges(&self) -> &Vec<Transition>{
        &self.edges
    }

    pub fn from_adj_matrix(data: &DMatrix<f64>, nodes: &Vec<State>) -> Self{
        let mut graph = Self::new();
        let progress = ProgressBar::new(data.nrows() as u64).with_message("Building graph");
        for i in 0..nodes.len(){
            let noderef = nodes[i].clone();
            for j in 0..nodes.len() {
                if data[(i,j)] != 0.0{
                    graph.add_edge(Transition::from_value(noderef.clone(), nodes[j].clone(),data[(i,j)]));
                }
            }
            progress.inc(1);
        }
        graph.nodes = nodes.clone();
        progress.finish();
        graph
    }

}

pub struct ChainGenerator {
    node_generator: NodeGenerator,
    graph : Graph,
    frontier: VecDeque<State>,
    ordered: Vec<State>,
}

impl ChainGenerator {
    pub fn new() -> Self {
        ChainGenerator {
            graph: Graph::new(),
            node_generator: NodeGenerator::new(),
            frontier: VecDeque::new(),
            ordered: Vec::new(),
        }
    }

    fn next(&mut self) {
        let mut noderef = self.frontier.pop_front().unwrap();
        if self.ordered.contains(&noderef) {
            return;
        }
        for node in &self.node_generator.generated {
            let mut tr = Transition::new(noderef, node.clone());
            if tr.is_valid() {
                self.frontier.push_back(node.clone());
                self.graph.add_edge(tr);
            }
        }
        if !self.ordered.contains(&noderef) {
            self.ordered.push(noderef);
        }
    }

    pub fn generate(&mut self, starter: State) -> &mut Self {
        let mut progress = ProgressBar::new_spinner().with_message("generating nodes");
        self.node_generator.generate();
        self.frontier.push_back(starter);
        while !self.frontier.is_empty() {
            self.next();
            progress.inc(1);
        }
        self.graph.nodes = self.ordered.clone();
        self
    }

    pub fn ordered(&self) -> &Vec<State> {
        &self.ordered
    }

    pub fn graph(&self)-> Graph{
        return self.graph.clone();
    }


}

#[cfg(test)]
mod tests {
    use std::fmt::Debug;


    use crate::transition::TransitionType;

    use super::*;

    #[test]
    fn test_node_generator() {
        let mut generator = NodeGenerator::new();
        Params::instance().numclients = 3;
        generator.generate();
        assert!(generator.generated.len() == 30);
    }

    #[test]
    fn test_graph_generator() {
        let mut generator = ChainGenerator::new();
        Params::instance().numclients = 3;
        generator.generate(State::new(3, 0, 0, 0, 0));
        assert!(generator.ordered.len() == 30);
        let mut mat = generator.graph().adj_matrix();
        assert!(mat.row(0).len() == 30);
        assert!(mat.row(0).iter().any(|x| return *x > 0.0));
    }
}

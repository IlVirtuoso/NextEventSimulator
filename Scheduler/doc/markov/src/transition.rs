use core::panic;

use nalgebra::{Vector, Vector1};
use num_traits::FromPrimitive;

use crate::{
    parameters::Params,
    state::{State, StateDescriptor},
};

#[derive(PartialEq, Clone)]
pub enum TransitionType {
    Absolute = -2,
    Unknown = -1,
    CpuToIo1 = 1,
    CpuToIo2 = 2,
    CpuToSelf = 3,
    CpuToDelay = 4,
    DelayToCpu = 5,
    Io1ToCpu = 6,
    Io2ToCpu = 7,
}

#[derive(Clone)]
pub struct Transition {
    t_type: TransitionType,
    head: State,
    tail: State,
    prob: f64,
}

impl Transition {
    pub fn new(head: State, tail: State) -> Self {
        Transition {
            head,
            t_type: TransitionType::Unknown,
            tail,
            prob: 0.0,
        }
    }

    pub fn from_value(head: State, tail: State, value: f64) -> Self {
        Transition {
            t_type: TransitionType::Absolute,
            head: head,
            tail: tail,
            prob: value,
        }
    }

    pub fn head(&self) -> &State {
        &self.head
    }

    pub fn tail(&self) -> &State {
        &self.tail
    }

    pub fn t_type(&self) -> &TransitionType {
        return &self.t_type;
    }
    pub fn detect(&mut self) {
        if self.prob != 0.0 {
            return;
        }
        let diff = self.tail - self.head;
        if diff.iter().all(|&x| x == 0) && self.head.n_cpu > 0 && self.tail.n_cpu > 0 {
            self.t_type = TransitionType::CpuToSelf;
            return;
        }
        if diff.max() > 1 || diff.min() < -1 {
            return;
        }
        let argmin: StateDescriptor = StateDescriptor::from_usize(diff.argmin().0).unwrap();
        let argmax = StateDescriptor::from_usize(diff.argmax().0).unwrap();
        match argmin {
            StateDescriptor::Ndelay => {
                if argmax == StateDescriptor::Ncpu {
                    self.t_type = TransitionType::DelayToCpu;
                }
            }
            StateDescriptor::Ncpu => match argmax {
                StateDescriptor::Ndelay => self.t_type = TransitionType::CpuToDelay,
                StateDescriptor::Ncpu => self.t_type = TransitionType::CpuToSelf,
                StateDescriptor::Nio1 => self.t_type = TransitionType::CpuToIo1,
                StateDescriptor::Nio2 => self.t_type = TransitionType::CpuToIo2,
            },
            StateDescriptor::Nio1 => {
                if argmax == StateDescriptor::Ncpu {
                    self.t_type = TransitionType::Io1ToCpu
                }
            }
            StateDescriptor::Nio2 => {
                if argmax == StateDescriptor::Ncpu {
                    self.t_type = TransitionType::Io2ToCpu
                }
            }
        }
    }

    pub fn is_valid(&mut self) -> bool {
        if self.t_type == TransitionType::Unknown {
            self.detect();
        }
        self.t_type != TransitionType::Unknown
    }

    pub fn probability(&mut self) -> f64 {
        if self.prob != 0.0 {
            return self.prob;
        } else {
            self.prob = {
                match self.t_type {
                    TransitionType::Absolute => self.prob,
                    TransitionType::Unknown => panic!("Don't use p on invalid transitions"),
                    TransitionType::CpuToIo1 => self.cpu_to_io(),
                    TransitionType::CpuToIo2 => self.cpu_to_io(),
                    TransitionType::CpuToSelf => self.cpu_to_self(),
                    TransitionType::CpuToDelay => self.cpu_to_delay(),
                    TransitionType::DelayToCpu => self.delay_to_cpu(),
                    TransitionType::Io1ToCpu => self.io_to_cpu(),
                    TransitionType::Io2ToCpu => self.io_to_cpu(),
                }
            }
        }
        self.prob
    }

    fn cpu_onarrive_stage_selector(&self) -> f64 {
        debug_assert!(self.head.n_cpu <= self.tail.n_cpu);
        if self.head.n_cpu > 0 {
            return 1.0;
        } else if self.tail.cpu_stage == 1 {
            return Params::instance().alpha;
        } else if self.tail.cpu_stage == 2 {
            return Params::instance().beta;
        }
        panic!("invalid cpu stage")
    }

    fn cpu_onleave_stage_selector(&self) -> f64 {
        debug_assert!(self.head.n_cpu >= self.tail.n_cpu);
        if self.tail.n_cpu == 0 {
            return 1.0;
        } else if self.tail.cpu_stage == 1 {
            return Params::instance().alpha;
        } else if self.tail.cpu_stage == 2 {
            return Params::instance().beta;
        }
        panic!("invalid cpu stage")
    }

    fn cpu_leave(&self) -> f64 {
        let service = if self.head.cpu_stage == 1 {
            Params::instance().u1
        } else {
            Params::instance().u2
        };
        return (1.0 / (service as f64)) * self.cpu_onleave_stage_selector();
    }

    // cpu handling
    fn cpu_to_self(&self) -> f64 {
        debug_assert!(self.head.n_cpu == self.tail.n_cpu);
        self.cpu_leave() * Params::instance().qouts
            + 1.0 / (Params::instance().timeslice) * self.cpu_onleave_stage_selector()
    }

    fn cpu_to_io(&self) -> f64 {
        debug_assert!(
            self.head.n_cpu > self.tail.n_cpu
                && if self.t_type == TransitionType::CpuToIo1 {
                    self.head.n_io1 < self.tail.n_io1
                } else {
                    self.head.n_io2 < self.tail.n_io2
                },
            "{}->{}",
            self.head,
            self.tail
        );

        self.cpu_leave()
            * if self.t_type == TransitionType::CpuToIo1 {
                Params::instance().qio1
            } else {
                Params::instance().qio2
            }
    }

    fn cpu_to_delay(&self) -> f64 {
        debug_assert!(
            self.head.n_cpu > self.tail.n_cpu && self.head.n_delay < self.tail.n_delay,
            "{}->{}",
            self.head,
            self.tail
        );
        self.cpu_leave() * Params::instance().qoutd
    }

    //io handling

    fn io_to_cpu(&self) -> f64 {
        debug_assert!(
            self.head.n_cpu < self.tail.n_cpu
                && if self.t_type == TransitionType::Io1ToCpu {
                    self.head.n_io1 > self.tail.n_io1
                } else {
                    self.head.n_io2 > self.tail.n_io2
                },
            "{}->{}",
            self.head,
            self.tail
        );

        let service = if self.t_type == TransitionType::Io1ToCpu {
            Params::instance().sio1
        } else {
            Params::instance().sio2
        };
        (1.0 / service) * self.cpu_onarrive_stage_selector()
    }

    // delay
    fn delay_to_cpu(&self) -> f64 {
        debug_assert!(self.head.n_cpu < self.tail.n_cpu && self.head.n_delay > self.tail.n_delay);
        (1.0 / Params::instance().thinktime)
            * self.cpu_onarrive_stage_selector()
            * self.head.n_delay as f64
    }
}

#[cfg(test)]
mod tests {
    use std::process::Output;

    use super::*;

    #[test]
    fn test_transition_instance() {
        let mut tr = Transition::new(State::new(3, 0, 0, 0, 0), State::new(2, 1, 0, 0, 1));
        assert!(tr.is_valid());
        assert!(tr.t_type == TransitionType::DelayToCpu);
    }

    fn verify_state_transition(s1: State, s2: State, expected: f64) {
        let mut tr = Transition::new(s1, s2);
        assert!(tr.is_valid(), "invalid transition");
        assert!(
            tr.probability() == expected,
            "mismatch expected:{} value:{}",
            expected,
            tr.probability()
        );
    }

    #[test]
    fn test_transition_probability() {
        let params = Params::instance();
        verify_state_transition(
            State::new(3, 0, 0, 0, 0),
            State::new(2, 1, 0, 0, 1),
            (1.0 / Params::instance().thinktime) * 3.0 * Params::instance().alpha,
        );

        verify_state_transition(
            State::new(1, 2, 0, 0, 1),
            State::new(1, 2, 0, 0, 2),
            (1.0 / (Params::instance().u1 * Params::instance().beta)) * Params::instance().qouts
                + 1.0 / (Params::instance().timeslice * Params::instance().beta),
        );

        verify_state_transition(
            State::new(2, 1, 0, 0, 1),
            State::new(2, 1, 0, 0, 2),
            1.0 / (params.u1 * params.beta) * params.qouts + 1.0 / (params.timeslice * params.beta),
        )
    }
}

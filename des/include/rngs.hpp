/* -----------------------------------------------------------------------
 * Name            : rngs.h  (header file for the library file rngs.c)
 * Author          : Steve Park & Dave Geyer, DrFaust
 * Language        : C++
 * -----------------------------------------------------------------------
 */
#pragma once

#include <functional>
#include <memory>
#include <vector>
#define MODULUS 2147483647 /* DON'T CHANGE THIS VALUE                  */
#define MULTIPLIER 48271   /* DON'T CHANGE THIS VALUE                  */
#define CHECK 399268537    /* DON'T CHANGE THIS VALUE                  */
#define STREAMS 256        /* # of streams, DON'T CHANGE THIS VALUE    */
#define A256 22925         /* jump multiplier, DON'T CHANGE THIS VALUE */
#define DEFAULT 123456789  /* initial seed, use 0 < DEFAULT < MODULUS  */

class RandomStream;

struct BaseStream
{
    virtual double operator()() = 0;
};

class VariableStream : public BaseStream
{
    int stream;
    std::function<double(RandomStream &)> _lambda;

  public:
    double operator()() override;
    VariableStream(int stream, std::function<double(RandomStream &)> lambda);
};

struct CompositionStream : public BaseStream
{
    std::vector<std::function<double(RandomStream &)>> _generators{};
    std::vector<double> _alpha;
    double operator()() override;
    int _stream;
    template <typename... F>
    CompositionStream(int stream, std::vector<double> weights, F &&...fncs)
        : _generators({fncs...}), _alpha(weights), _stream(stream)
    {
    }
};

struct Router
{
    std::vector<int> _indexes;
    std::vector<double> _p;
    int operator()();
    int _stream{};
    Router(int stream, std::vector<double> probs, std::vector<int> indexes);
};

class RandomStream
{
    long seed[STREAMS] = {DEFAULT}; /* current state of each stream   */
    int stream = 0;                 /* stream index, 0 is the default */
    int initialized = 0;            /* test for stream initialization */
    int generatedStreams = 0;

  public:
    bool antitethic = false;

    RandomStream()
    {
    }
    double Random(void);
    void PlantSeeds(long x);
    void GetSeed(long *x);
    void PutSeed(long x);
    void SelectStream(int index);
    void SetAntitetich(bool value)
    {
        antitethic = value;
    }
    static RandomStream &Global();
    std::unique_ptr<VariableStream> GetStream(std::function<double(RandomStream &)> lambda);
};

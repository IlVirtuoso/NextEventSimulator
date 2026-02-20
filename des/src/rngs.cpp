/* -------------------------------------------------------------------------
 * This is an ANSI C library for multi-stream random number generation.
 * The use of this library is recommended as a replacement for the ANSI C
 * rand() and srand() functions, particularly in simulation applications
 * where the statistical 'goodness' of the random number generator is
 * important.  The library supplies 256 streams of random numbers; use
 * SelectStream(s) to switch between streams indexed s = 0,1,...,255.
 *
 * The streams must be initialized.  The recommended way to do this is by
 * using the function PlantSeeds(x) with the value of x used to initialize
 * the default stream and all other streams initialized automatically with
 * values dependent on the value of x.  The following convention is used
 * to initialize the default stream:
 *    if x > 0 then x is the state
 *    if x < 0 then the state is obtained from the system clock
 *    if x = 0 then the state is to be supplied interactively.
 *
 * The generator used in this library is a so-called 'Lehmer random number
 * generator' which returns a pseudo-random number uniformly distributed
 * 0.0 and 1.0.  The period is (m - 1) where m = 2,147,483,647 and the
 * smallest and largest possible values are (1 / m) and 1 - (1 / m)
 * respectively.  For more details see:
 *
 *       "Random Number Generators: Good Ones Are Hard To Find"
 *                   Steve Park and Keith Miller
 *              Communications of the ACM, October 1988
 *
 * Name            : rngs.c  (Random Number Generation - Multiple Streams)
 * Authors         : Steve Park & Dave Geyer
 * Language        : C++
 * Latest Revision : 09-22-98
 * -------------------------------------------------------------------------
 */

#include "rngs.hpp"
#include <memory>
#include <stdio.h>
#include <time.h>
#include <vector>

double RandomStream::Random(void)
/* ----------------------------------------------------------------
 * Random returns a pseudo-random real number uniformly distributed
 * between 0.0 and 1.0.
 * ----------------------------------------------------------------
 */
{
    const long Q = MODULUS / MULTIPLIER;
    const long R = MODULUS % MULTIPLIER;
    long t;

    t = MULTIPLIER * (seed[stream] % Q) - R * (seed[stream] / Q);
    if (t > 0)
        seed[stream] = t;
    else
        seed[stream] = t + MODULUS;
    double n =  ((double)seed[stream] / MODULUS);
    return antitethic? 1-n:n;
}

void RandomStream::PlantSeeds(long x)
/* ---------------------------------------------------------------------
 * Use this function to set the state of all the random number generator
 * streams by "planting" a sequence of states (seeds), one per stream,
 * with all states dictated by the state of the default stream.
 * The sequence of planted states is separated one from the next by
 * 8,367,782 calls to Random().
 * ---------------------------------------------------------------------
 */
{
    const long Q = MODULUS / A256;
    const long R = MODULUS % A256;
    int j;
    int s;

    initialized = 1;
    s = stream;      /* remember the current stream */
    SelectStream(0); /* change to stream 0          */
    PutSeed(x);      /* set seed[0]                 */
    stream = s;      /* reset the current stream    */
    for (j = 1; j < STREAMS; j++)
    {
        x = A256 * (seed[j - 1] % Q) - R * (seed[j - 1] / Q);
        if (x > 0)
            seed[j] = x;
        else
            seed[j] = x + MODULUS;
    }
}

void RandomStream::PutSeed(long x)
/* ---------------------------------------------------------------
 * Use this function to set the state of the current random number
 * generator stream according to the following conventions:
 *    if x > 0 then x is the state (unless too large)
 *    if x < 0 then the state is obtained from the system clock
 *    if x = 0 then the state is to be supplied interactively
 * ---------------------------------------------------------------
 */
{
    char ok = 0;
    if (x > 0)
        x = x % MODULUS; /* correct if x is too large  */
    if (x <= 0)
        x = ((unsigned long)time((time_t *)NULL)) % MODULUS;
    seed[stream] = x;
}

void RandomStream::GetSeed(long *x)
/* ---------------------------------------------------------------
 * Use this function to get the state of the current random number
 * generator stream.
 * ---------------------------------------------------------------
 */
{
    *x = seed[stream];
}

void RandomStream::SelectStream(int index)
/* ------------------------------------------------------------------
 * Use this function to set the current random number generator
 * stream -- that stream from which the next random number will come.
 * ------------------------------------------------------------------
 */
{
    stream = ((unsigned int)index) % STREAMS;
    if ((initialized == 0) && (stream != 0)) /* protect against        */
        PlantSeeds(DEFAULT);                 /* un-initialized streams */
}

RandomStream &RandomStream::Global()
{
    static RandomStream instance{};
    return instance;
}

std::unique_ptr<VariableStream> RandomStream::GetStream(std::function<double(RandomStream &)> lambda)
{
    return std::make_unique<VariableStream>(VariableStream(generatedStreams++, lambda));
}

VariableStream::VariableStream(int stream, std::function<double(RandomStream &)> lambda)
    : stream(stream), _lambda(lambda)
{
}

double VariableStream::operator()()
{
    RandomStream::Global().SelectStream(stream);
    return _lambda(RandomStream::Global());
}

int selector(std::vector<double> alpha)
{
    std::vector<double> A{alpha};
    for (int i = 1; i < A.size(); i++)
    {
        A[i] = A[i - 1] + alpha[i];
    }
    auto Y = RandomStream::Global().Random();
    auto r = 0;
    while (Y >= A[r])
        r++;
    return r;
}

double CompositionStream::operator()()
{
    RandomStream::Global().SelectStream(_stream);
    return _generators[selector(_alpha)](RandomStream::Global());
}


int Router::operator()() {
    RandomStream::Global().SelectStream(_stream);
    return _indexes[selector(_p)];
}

Router::Router(int stream, std::vector<double> probs, std::vector<int> indexes)
    : _stream(stream), _indexes(indexes), _p(probs)
{
}

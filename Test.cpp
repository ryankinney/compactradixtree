#include <cmath>
#include <gtest/gtest.h>
#include <set>
#include <sstream>
#include <string>
#include <tr1/memory>
#include <vector>
#include "UniqueNumberCounter.h"

using namespace std;
using namespace std::tr1;

namespace
{
    void TestSparseData(shared_ptr<IUniqueNumberAlgorithm> algorithm)
    {
        UniqueNumberCounter counter(algorithm, 3);
        counter.ProcessNumber("123");
        counter.ProcessNumber("456");
        counter.ProcessNumber("123");
        counter.ProcessNumber("457");
        counter.ProcessNumber("442");
        counter.ProcessNumber("441");
        EXPECT_EQ(5, counter.GetCount());
    }

    void TestFullData(shared_ptr<IUniqueNumberAlgorithm> algorithm)
    {
        const size_t numDigits(1);
        UniqueNumberCounter counter(algorithm, numDigits);
        for (size_t number = 0; number < numDigits * 10; ++number)
        {
            ostringstream out;
            out << setw(numDigits) << setfill('0') << number;
            counter.ProcessNumber(out.str());
        }
        EXPECT_EQ(numDigits * 10, counter.GetCount());
    }

    void TestAlgorithm(shared_ptr<IUniqueNumberAlgorithm> algorithm)
    {
        TestSparseData(algorithm);
        TestFullData(algorithm);
    }

    typedef vector<string> Dataset;

    size_t ProcessDataset(const size_t numDigits, const Dataset &dataset, shared_ptr<IUniqueNumberAlgorithm> algorithm)
    {
        UniqueNumberCounter counter(algorithm, numDigits);
        for (Dataset::const_iterator entry(dataset.begin()); entry != dataset.end(); ++entry)
            counter.ProcessNumber(*entry);
        return counter.GetCount();
    }
}

TEST(TestUniqueNumberCounter, NullAlgorithm)
{
    EXPECT_THROW(UniqueNumberCounter counter(shared_ptr<IUniqueNumberAlgorithm>(), 1),
                 runtime_error);
}

TEST(TestUniqueNumberCounter, InvalidNumDigits)
{
    shared_ptr<IUniqueNumberAlgorithm> algorithm(IUniqueNumberAlgorithm::CreateInstance(IUniqueNumberAlgorithm::Set));
    EXPECT_THROW(UniqueNumberCounter counter(algorithm, 0),
                 runtime_error);
}

TEST(TestUniqueNumberCounter, SetAlgorithmSmallDataSet)
{
    shared_ptr<IUniqueNumberAlgorithm> algorithm(IUniqueNumberAlgorithm::CreateInstance(IUniqueNumberAlgorithm::Set));
    TestAlgorithm(algorithm);
}


TEST(TestUniqueNumberCounter, CompactRadixTreeAlgorithmSmallDataSet)
{
    shared_ptr<IUniqueNumberAlgorithm> algorithm(IUniqueNumberAlgorithm::CreateInstance(IUniqueNumberAlgorithm::CompactRadixTree));
    TestAlgorithm(algorithm);
}

TEST(TestUniqueNumberCounter, LargeDataSet)
{
    // Generate a large data set
    Dataset dataset;
    const size_t maxDigits(static_cast<size_t>(log10(RAND_MAX)));
    for (size_t count(0); count < 1000000; ++count)
    {
        ostringstream out;
        out << setw(maxDigits) << setfill('0') << rand();
        string value(out.str());
        if (value.size() > maxDigits)
            value.resize(maxDigits);
        dataset.push_back(value);
    }

    size_t firstCount(0);
    {
        shared_ptr<IUniqueNumberAlgorithm> algorithm(IUniqueNumberAlgorithm::CreateInstance(IUniqueNumberAlgorithm::Set));
        firstCount = ProcessDataset(maxDigits, dataset, algorithm);
    }
    EXPECT_GT(firstCount, 0);

    size_t secondCount(0);
    {
        shared_ptr<IUniqueNumberAlgorithm> algorithm(IUniqueNumberAlgorithm::CreateInstance(IUniqueNumberAlgorithm::CompactRadixTree));
        secondCount = ProcessDataset(maxDigits, dataset, algorithm);
    }
    EXPECT_EQ(firstCount, secondCount);
}

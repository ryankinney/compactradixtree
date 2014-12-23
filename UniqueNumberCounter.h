#pragma once

#include <tr1/memory>

/**
 * \brief An algorithm that can be used to detect unique numbers in a large stream of numbers.
 */
class IUniqueNumberAlgorithm
{
public:
    /**
     * \brief The different implementations of this algortihm.
     */
    enum AlgorithmType
    {
       CompactRadixTree, /**< Implements the algorithm using a compact radix tree, which is slower but uses memory
                              more efficiently. */
       Set               /**< Implements the algorithm using a STL set, which is faster but uses more momory. */
    };

    /**
     * \brief Returns an instance of this interface given the specified algorithm implementation.
     *
     * @param[in] algorithmType Which algorithm to instantiate.
     *
     * @return Returns an algorithm according to the one specified by algorithmType.
     */
    static std::tr1::shared_ptr<IUniqueNumberAlgorithm> CreateInstance(const AlgorithmType algorithmType);

    /**
     * \brief Must be sub-classed.
     */
    virtual ~IUniqueNumberAlgorithm() {}

    /**
     * \brief Resets the algorithm to its initial state so that it 'forgets' all numbers it has seen so far.
     */
    virtual void Reset() = 0;

    /**
     * \brief Returns true if the specified number is unique and has not been encountered in the number stream.
     */
    virtual bool IsUnique(const std::string &number) = 0;
};

/**
 * \brief Uses an instance of an IUniqueNumberAlgorithm to detect unique numbers is a stream of numbers.
 */
class UniqueNumberCounter
{
public:
    /**
     * \brief Stores the parameters to be used in other methods, but otherwise has no side-effects.
     *
     * @param[in] algorithm         The algorithm this object should use for rememberingn numbers.
     * @param[in] numExpectedDigits The number of digits each number is expected to have.
     */
    UniqueNumberCounter(std::tr1::shared_ptr<IUniqueNumberAlgorithm> algorithm, const size_t numExpectedDigits);

    /**
     * \brief Processes a number from the number stream.
     */
    void ProcessNumber(const std::string &number);

    /**
     * \brief Returns the number of unique numbers encountered so far.
     */
    size_t GetCount() const { return m_Count; }

private:
    /**
     * \brief Checks a number to make sure it's valid (e.g. correct number of digits, is actually a number, etc...)
     *
     * @param[in] number The number to check.
     */
    void m_CheckNumber(const std::string &number) const;

    const size_t m_NumExpectedDigits;                         /**< Number of digits each number in the stream should contain. */
    std::tr1::shared_ptr<IUniqueNumberAlgorithm> m_Algorithm; /**< Algorithm to use to detect unique numbers */
    size_t m_Count;                                           /**< Number of unique numbers detected so far */
};

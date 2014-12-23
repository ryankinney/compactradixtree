#include "UniqueNumberCounter.h" // Main header

#include <iostream>
#include <list>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
using namespace std::tr1;

namespace
{
    /**
     * \brief Throws an exception containing the specified message.
     *
     * @param[in] message Message that should be included in the exception.
     */
    void RaiseError(const string &message)
    {
        throw runtime_error(message);
    }

    /**
     * \brief Implements the unique number algorithm using a compact radix tree, which is slower but
     *        uses memory more efficiently.
     */
    class CompactRadixTreeAlgorithm : public IUniqueNumberAlgorithm
    {
    public:
        /**
         * \brief Initializes the root node of the tree.
         */
        CompactRadixTreeAlgorithm() :
            m_Root(new Node)
        {
        }

        /**
         * \brief Deletes all nodes in the tree.
         */
        virtual void Reset()
        {
            m_Root.reset(new Node);
        }

        /**
         * \copydoc IUniqueNumberAlgorithm::IsUnique
         */
        virtual bool IsUnique(const string &value)
        {
            bool isUnique(false);
            string remainder(value);
            shared_ptr<Node> current(m_Root);
            while (!remainder.empty())
                current = current->Eat(remainder, isUnique);
            return isUnique;
        }

        /**
         * \brief Prints the contents of the tree to standard out.
         */
        void Print() const
        {
            cout << "PRINTING" << endl;
            if (m_Root.get() != NULL)
                m_Root->Print(0);
            cout << "DONE" << endl;
        }

    private:
        class Node;

        /**
         * \brief Represents a single edge in the tree.
         */
        class Edge
        {
        public:
            /**
             * \brief Intializes an edge and attaches the specified node.
             *
             * @param[in] value The string required to transition to the next node using this edge.
             * @param[in] next  Optional, The next node in the tree when following this edge. If NULL,
             *                  an empty node will be created.
             */
            Edge(const string &value, shared_ptr<Node> next = shared_ptr<Node>()) :
                m_Value(value),
                m_Next(next)
            {
                if (m_Next.get() == NULL)
                   m_Next.reset(new Node);
            }

            /**
             * \brief Returns the string required to transition to the next node using this edge.
             */
            const string &GetValue() const { return m_Value; }

            /**
             * \brief Returns the next node in the tree when following this edge.
             */
            const Node &GetNext() const { return *m_Next; }

            /**
             * \brief Returns the number of common characters between the value stored in this edge
             *        and the specified string.
             *
             * @param[in] remainder The string to compare against this value.
             *
             * @return Returns the number of common characters.
             */
            size_t GetNumCommonChars(const string &remainder) const
            {
                // Check arguments
                if (remainder.empty())
                    RaiseError("invalid remainder");

                // How many chacters are common between this edge and the new value
                size_t numCommonChars(0);
                for (; (numCommonChars < m_Value.size()) && (numCommonChars < remainder.size()); ++numCommonChars)
                    if (m_Value[numCommonChars] != remainder[numCommonChars])
                        break;
                return numCommonChars;
            }

            /**
             * \brief Eats as many characters as possible from the specified string by following this edge. The edge
             *        may need to be split in order to follow it. This function will not eat any characters if
             *        numCommonChars is 0.
             *
             * @param[in] numCommonChars Number of common characters between this node's value and the remainder parameter.
             * @param[in] remainder      Attempts to eat the starting characters in remainder by following this edge.
             * @param[in] isUnique       Returns true if the number has been determined to be unique.
             *
             * @return Returns the next node if this edge was followed. Otherwise, NULL is returned.
             */
            shared_ptr<Node> Eat(const size_t numCommonChars, string &remainder, bool &isUnique)
            {
                // Check arguments
                if ((numCommonChars == 0) || (numCommonChars > min(m_Value.size(), remainder.size())))
                    RaiseError("invalid numCommonChars");
                if (remainder.empty())
                    RaiseError("invalid remainder");
            
                shared_ptr<Node> next;
                if (numCommonChars > 0)
                {
                    if (numCommonChars == m_Value.size())
                    {
                        // This edge matches the beginning of remainder. Traverse the edge. No splitting is neeeded.
                        remainder = remainder.substr(numCommonChars);
                        next = m_Next;
                    }
                    else
                    {
                        // This edge contains some common characters, but not all characters are common so it needs to
                        // split.
                        const string &commonValue(m_Value.substr(0, numCommonChars));
                        const string &firstChildValue(m_Value.substr(numCommonChars));
                        const string &secondChildValue(remainder.substr(numCommonChars));

                        // Create two new edges
                        shared_ptr<Edge> firstEdge(new Edge(firstChildValue, m_Next));
                        shared_ptr<Edge> secondEdge(new Edge(secondChildValue));

                        // Fix this edge
                        m_Value = commonValue;
                        m_Next.reset(new Node(firstEdge, secondEdge));

                        remainder = "";
                        isUnique = true;
                    }
                }
                return next;
            }

        private:
            string m_Value;          /**< String required to transition to the next node using this edge. */
            shared_ptr<Node> m_Next; /**< Next node in the tree when following this edge. */
        };

        /**
         * \brief How edges are ordered in a node can dramatically affect its performance. This class orders
         *        edges in an unordered linked list, yeilding O(n) performance.
         */
        class UnorderedEdges
        {
        public:
            /**
             * \brief STL container used to store edges.
             */
            typedef list<shared_ptr<Edge> > Container;

            /**
             * \brief Returns the STL container used to store edges.
             */
            const Container &GetContainer() const { return m_Container; }

            /**
             * \brief Returns the edge stored in the specified iterator. This method abstracts out the differences
             *        between a map iterator (which contains a pair) and some other kind of iterator (which does not).
             *
             * @param[in] iter Get the edge from this iterator.
             *
             * @return Returns the edge stored in iter.
             */
            const Edge &GetEdge(Container::const_iterator iter) const { return **iter; }

            /**
             * \brief Adds an edge to the underlying container.
             *
             * @param[in] edge The edge to add.
             */
            void Add(shared_ptr<Edge> edge)
            {
                m_Container.push_back(edge);
            }

            /**
             * \brief Finds an edge that has common characters with remainder.
             *
             * @param[in] remainder Look for an edge that has common characters with this string.
             *
             * @return If an edge with common characters was found, then first will contain the number of common
             *         characters and second will contain the edge. If an edge was not found, then second will be
             *         NULL.
             */
            pair<size_t, shared_ptr<Edge> > Find(const string &remainder) const
            { 
                pair<size_t, shared_ptr<Edge> > ret;
                for (Container::const_iterator iter(m_Container.begin()); iter != m_Container.end(); ++iter)
                {
                    shared_ptr<Edge> edge(*iter);
                    ret.first = edge->GetNumCommonChars(remainder);
                    if (ret.first > 0)
                    {
                        ret.second = edge;
                        break;
                    }
                }
                return ret;
            }

        private:
            Container m_Container; /**< The STL container used to store edges. */
        };

        /**
         * \brief How edges are ordered in a node can dramatically affect its performance. This class orders
         *        edges in an STL map, yeilding O(log n) performance.
         */
        class OrderedEdges
        {
        public:
            /**
             * \brief STL container used to store edges.
             */
            typedef map<string, shared_ptr<Edge> > Container;

            /**
             * \brief Returns the STL container used to store edges.
             */
            const Container &GetContainer() const { return m_Container; }

            /**
             * \brief Returns the edge stored in the specified iterator. This method abstracts out the differences
             *        between a map iterator (which contains a pair) and some other kind of iterator (which does not).
             *
             * @param[in] iter Get the edge from this iterator.
             *
             * @return Returns the edge stored in iter.
             */
            const Edge &GetEdge(Container::const_iterator iter) const { return *iter->second; }

            /**
             * \brief Adds an edge to the underlying container.
             *
             * @param[in] edge The edge to add.
             */
            void Add(shared_ptr<Edge> edge)
            {
                m_Container.insert(make_pair(edge->GetValue(), edge));
            }

            /**
             * \brief Finds an edge that has common characters with remainder.
             *
             * @param[in] remainder Look for an edge that has common characters with this string.
             *
             * @return If an edge with common characters was found, then first will contain the number of common
             *         characters and second will contain the edge. If an edge was not found, then second will be
             *         NULL.
             */
            pair<size_t, shared_ptr<Edge> > Find(const string &remainder) const
            {
                pair<size_t, shared_ptr<Edge> > ret;
                if (!m_Container.empty())
                {
                    // upper_bound may return the edge that is needed or may return the edge after the edge
                    // that is needed. For example, if remainder is 'hello' and there is an edge for 'hea' then
                    // upper_bound will return whatever comes after 'hea'
                    Container::const_iterator iter(m_Container.upper_bound(remainder));
                    if (iter != m_Container.end())
                    {
                        shared_ptr<Edge> edge(iter->second);
                        ret.first = edge->GetNumCommonChars(remainder);
                        if (ret.first > 0)
                            ret.second = edge;
                    }
                    if (ret.first == 0)
                    {
                        if (iter-- != m_Container.begin())
                        {
                            shared_ptr<Edge> edge(iter->second);
                            ret.first = edge->GetNumCommonChars(remainder);
                            if (ret.first > 0)
                               ret.second = edge;
                        }
                    }
                }
                return ret;
            }

        private:
            Container m_Container; /**< The STL container used to store edges. */
        };

        /**
         * \brief How edges are ordered in a node can dramatically affect its performance. This class orders
         *        edges in a fixed size array so that they can be directly indexed, yeilding O(1) performance.
         */
        class IndexedEdges
        {
        public:
            /**
             * \brief STL container used to store edges.
             */
            typedef vector<shared_ptr<Edge> > Container;

            /**
             * \brief Initializes the STL container to hold up to 10 edges (corresponding to the digits 0-9).
             */
            IndexedEdges() :
                m_Container(10)
            {
            }

            /**
             * \brief Returns the STL container used to store edges.
             */
            const Container &GetContainer() const { return m_Container; }

            /**
             * \brief Returns the edge stored in the specified iterator. This method abstracts out the differences
             *        between a map iterator (which contains a pair) and some other kind of iterator (which does not).
             *
             * @param[in] iter Get the edge from this iterator.
             *
             * @return Returns the edge stored in iter.
             */
            const Edge &GetEdge(Container::const_iterator iter) const { return **iter; }

            /**
             * \brief Adds an edge to the underlying container.
             *
             * @param[in] edge The edge to add.
             */
            void Add(shared_ptr<Edge> edge)
            {
                m_Container[edge->GetValue()[0] - '0'] = edge;
            }

            /**
             * \brief Finds an edge that has common characters with remainder.
             *
             * @param[in] remainder Look for an edge that has common characters with this string.
             *
             * @return If an edge with common characters was found, then first will contain the number of common
             *         characters and second will contain the edge. If an edge was not found, then second will be
             *         NULL.
             */
            pair<size_t, shared_ptr<Edge> > Find(const string &remainder) const
            {
                pair<size_t, shared_ptr<Edge> > ret;
                ret.second = m_Container[remainder[0] - '0'];
                if (ret.second.get() != NULL)
                    ret.first = ret.second->GetNumCommonChars(remainder);
                return ret;
            }

        private:
            Container m_Container; /**< The STL container used to store edges. */
        };

        /**
         * \brief Represents a node in the tree. Each node can hold an arbitrary number of edges.
         */
        class Node
        {
        public:
            /**
             * \brief Creates a node with no edges.
             */
            Node()
            {
            }

            /**
             * \brief Creates a node with two edges.
             *
             * @param[in] The first edge in the node.
             * @param[in] The secondn edge in the node.
             */
            Node(shared_ptr<Edge> firstEdge, shared_ptr<Edge> secondEdge)
            {
                if ((firstEdge.get() == NULL) || (secondEdge.get() == NULL))
                    RaiseError("invalid edge");
                m_Edges.Add(firstEdge);
                m_Edges.Add(secondEdge);
            }

            /**
             * \brief Eats characters from the begining of remainder by transitioning to the next
             *        level node. A new edge and node may be created if one does not already exist.
             *
             * @param[in,out] remainder Eats charaters at the beginning of this string.
             * @param[in,out] isUnique  Returns true if this string is known to be unique.
             *
             * @return Returns the next node encountered after following an edge.
             */
            shared_ptr<Node> Eat(string &remainder, bool &isUnique)
            {
                // Check arguments
                if (remainder.empty())
                    RaiseError("invalid remainder");

                shared_ptr<Node> next;
                const pair<size_t, shared_ptr<Edge> > &ret(m_Edges.Find(remainder));
                if (ret.first > 0)
                    next = ret.second->Eat(ret.first, remainder, isUnique);

                if ((next.get() == NULL) && !remainder.empty())
                {
                    shared_ptr<Edge> newEdge(new Edge(remainder));
                    m_Edges.Add(newEdge);
                    remainder = "";
                    isUnique = true;
                }
                return next;
            }

            /**
             * \brief Prints the contents of this node and all child nodes.
             *
             * @param[in] depth The depth of indentation to use when printing this node.
             */
            void Print(const size_t depth) const
            {
                const string indent(2 * depth, ' ');
                const Edges::Container &container(m_Edges.GetContainer());
                for (Edges::Container::const_iterator iter(container.begin()); iter != container.end(); ++iter)
                {
                    const Edge &edge(m_Edges.GetEdge(iter));
                    cout << indent << "edge=" << edge.GetValue() << endl;
                    edge.GetNext().Print(depth + 1);
                }
            }

        private:
            /**
             * \brief Which edges colection to use.
             */
            typedef IndexedEdges Edges;

            Edges m_Edges; /**< Stores the edges for this node. */
        };

        shared_ptr<Node> m_Root; /**< Stores the root node for the tree. */
    };

    /**
     * \brief Implements the unique number algorithm using an STL set, which is faster but
     *        uses more memory.
     */
    class SetAlgorithm : public IUniqueNumberAlgorithm
    {
    public:
        /**
         * \copydoc IUniqueNumberAlgorithm::Reset
         */
        virtual void Reset()
        {
            m_Numbers.clear();
        }

        /**
         * \copydoc IUniqueNumberAlgorithm::IsUnique
         */
        virtual bool IsUnique(const string &number)
        {
            const pair<Numbers::iterator, bool> &ret(m_Numbers.insert(number));
            return ret.second;
        }

    private:
        /**
         * \brief Represent the unique numbers as an STL set of strings.
         */
        typedef set<string> Numbers;

        Numbers m_Numbers; /**< Set of unique numbers found in the stream */
    };
}

shared_ptr<IUniqueNumberAlgorithm> IUniqueNumberAlgorithm::CreateInstance(const AlgorithmType algorithmType)
{
    shared_ptr<IUniqueNumberAlgorithm> algorithm;
    switch (algorithmType)
    {
        case CompactRadixTree:
            algorithm.reset(new CompactRadixTreeAlgorithm);
            break;
        case Set:
            algorithm.reset(new SetAlgorithm);
            break;
        default:
            RaiseError("Invalid algorithmType");
    }
    return algorithm;
}

UniqueNumberCounter::UniqueNumberCounter(shared_ptr<IUniqueNumberAlgorithm> algorithm, const size_t numExpectedDigits) :
    m_Algorithm(algorithm),
    m_NumExpectedDigits(numExpectedDigits),
    m_Count(0)
{
    // Check arguments
    if (m_Algorithm.get() == NULL)
       RaiseError("NULL ptr");
    if (m_NumExpectedDigits <= 0)
       RaiseError("numExpectedDigits cannot be zero");

    // Reset the algorithm back to its intial state in case it's being reused
    m_Algorithm->Reset();
}

void UniqueNumberCounter::ProcessNumber(const string &number)
{
    // Check arguments
    m_CheckNumber(number);

    // If the number is unique, then increment the count
    if (m_Algorithm->IsUnique(number))
        m_Count++;
}

void UniqueNumberCounter::m_CheckNumber(const string &number) const
{
    if (number.size() != m_NumExpectedDigits)
        RaiseError("Invalid number of digits");
    for (string::const_iterator ch(number.begin()); ch != number.end(); ch++)
        if (!isdigit(*ch))
            RaiseError("Not a number");
}

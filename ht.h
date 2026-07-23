#ifndef HT_H
#define HT_H
#include <vector>
#include <iostream>
#include <cmath>
#include <functional>
#include <stdexcept>
#include <utility>

typedef size_t HASH_INDEX_T;


// Complete - Base Prober class
template <typename KeyType>
struct Prober {
    // Data members
    HASH_INDEX_T start_;    // initial hash location, h(k)
    HASH_INDEX_T m_;        // table size
    size_t numProbes_;      // probe attempts for statistic tracking
    static const HASH_INDEX_T npos = (HASH_INDEX_T)-1; // used to indicate probing failed
    void init(HASH_INDEX_T start, HASH_INDEX_T m, const KeyType& key)
    {
        (void) key;
        start_ = start;
        m_ = m;
        numProbes_ = 0;
    }
    HASH_INDEX_T next() {
        throw std::logic_error("Not implemented...should use derived class");
    }
};

// Linear probing
template <typename KeyType>
struct LinearProber : public Prober<KeyType> {
    HASH_INDEX_T next()
    {
        if(this->numProbes_ >= this->m_) {
            return this->npos;
        }
        HASH_INDEX_T loc = (this->start_ + this->numProbes_) % this->m_;
        this->numProbes_++;
        return loc;
    }
};

// Double hashing
template <typename KeyType, typename Hash2>
struct DoubleHashProber : public Prober<KeyType>
{
    Hash2 h2_;
    HASH_INDEX_T dhstep_;

    static const HASH_INDEX_T DOUBLE_HASH_MOD_VALUES[];
    static const int DOUBLE_HASH_MOD_SIZE;

private:
    HASH_INDEX_T findModulusToUseFromTableSize(HASH_INDEX_T currTableSize)
    {
        HASH_INDEX_T modulus = DOUBLE_HASH_MOD_VALUES[0];
        for(int i = 0;
            i < DOUBLE_HASH_MOD_SIZE && DOUBLE_HASH_MOD_VALUES[i] < currTableSize;
            i++)
        {
            modulus = DOUBLE_HASH_MOD_VALUES[i];
        }
        return modulus;
    }

public:
    DoubleHashProber(const Hash2& h2 = Hash2()) : h2_(h2), dhstep_(1) {}

    void init(HASH_INDEX_T start, HASH_INDEX_T m, const KeyType& key)
    {
        Prober<KeyType>::init(start, m, key);
        HASH_INDEX_T modulus = findModulusToUseFromTableSize(m);
        dhstep_ = modulus - (h2_(key) % modulus);
    }

    HASH_INDEX_T next()
    {
        if(this->numProbes_ >= this->m_) {
            return this->npos;
        }
        HASH_INDEX_T loc =
            (this->start_ + this->numProbes_ * dhstep_) % this->m_;
        this->numProbes_++;
        return loc;
    }
};

// Initialization of static array (do not alter)
template <typename KeyType, typename Hash2>
const HASH_INDEX_T DoubleHashProber<KeyType, Hash2>::DOUBLE_HASH_MOD_VALUES[] =
{
    7, 19, 43, 89, 193, 389, 787, 1583, 3191, 6397, 12841, 25703, 51431, 102871,
    205721, 411503, 823051, 1646221, 3292463, 6584957, 13169963, 26339921, 52679927,
    105359939, 210719881, 421439749, 842879563, 1685759113
};

template <typename KeyType, typename Hash2>
const int DoubleHashProber<KeyType, Hash2>::DOUBLE_HASH_MOD_SIZE =
    sizeof(DoubleHashProber<KeyType, Hash2>::DOUBLE_HASH_MOD_VALUES)/sizeof(HASH_INDEX_T);


// Hash Table Interface
template<
    typename K,
    typename V,
    typename Prober = LinearProber<K>,
    typename Hash = std::hash<K>,
    typename KEqual = std::equal_to<K> >
class HashTable
{
public:
    typedef K KeyType;
    typedef V ValueType;
    typedef std::pair<KeyType, ValueType> ItemType;
    typedef Hash Hasher;

    struct HashItem {
        ItemType item;
        bool deleted;
        HashItem(const ItemType& newItem) : item(newItem), deleted(false) {}
    };

    HashTable(
        double resizeAlpha = 0.4,
        const Prober& prober = Prober(),
        const Hasher& hash = Hasher(),
        const KEqual& kequal = KEqual());

    ~HashTable();

    bool empty() const;
    size_t size() const;
    void insert(const ItemType& p);
    void remove(const KeyType& key);

    ItemType const * find(const KeyType& key) const;
    ItemType * find(const KeyType& key);

    const ValueType& at(const KeyType& key) const;
    ValueType& at(const KeyType& key);
    const ValueType& operator[](const KeyType& key) const;
    ValueType& operator[](const KeyType& key);

    void reportAll(std::ostream& out) const;
    void clearTotalProbes() { totalProbes_ = 0; }
    size_t totalProbes() const { return totalProbes_; }

private:
    HashItem * internalFind(const KeyType& key) const;
    HASH_INDEX_T probe(const KeyType& key) const;

    static const HASH_INDEX_T npos = Prober::npos;
    void resize();

    std::vector<HashItem*> table_;
    Hasher hash_;
    KEqual kequal_;
    mutable Prober prober_;
    mutable size_t totalProbes_;
    static const HASH_INDEX_T CAPACITIES[];
    HASH_INDEX_T mIndex_;

    double resizeAlpha_;
    size_t size_;       // number of active (not deleted) items
    size_t occupied_;   // number of allocated buckets, including deleted items
};


// Static array of prime table sizes
template<typename K, typename V, typename Prober, typename Hash, typename KEqual>
const HASH_INDEX_T HashTable<K,V,Prober,Hash,KEqual>::CAPACITIES[] =
{
    11, 23, 47, 97, 197, 397, 797, 1597, 3203, 6421, 12853, 25717, 51437, 102877,
    205759, 411527, 823117, 1646237, 3292489, 6584983, 13169977, 26339969, 52679969,
    105359969, 210719881, 421439783, 842879579, 1685759167
};

template<typename K, typename V, typename Prober, typename Hash, typename KEqual>
HashTable<K,V,Prober,Hash,KEqual>::HashTable(
    double resizeAlpha, const Prober& prober, const Hasher& hash, const KEqual& kequal)
    : table_(CAPACITIES[0], nullptr),
      hash_(hash),
      kequal_(kequal),
      prober_(prober),
      totalProbes_(0),
      mIndex_(0),
      resizeAlpha_(resizeAlpha),
      size_(0),
      occupied_(0)
{
}

template<typename K, typename V, typename Prober, typename Hash, typename KEqual>
HashTable<K,V,Prober,Hash,KEqual>::~HashTable()
{
    for(HASH_INDEX_T i = 0; i < table_.size(); ++i)
    {
        delete table_[i];
    }
}

template<typename K, typename V, typename Prober, typename Hash, typename KEqual>
bool HashTable<K,V,Prober,Hash,KEqual>::empty() const
{
    return size_ == 0;
}

template<typename K, typename V, typename Prober, typename Hash, typename KEqual>
size_t HashTable<K,V,Prober,Hash,KEqual>::size() const
{
    return size_;
}

template<typename K, typename V, typename Prober, typename Hash, typename KEqual>
void HashTable<K,V,Prober,Hash,KEqual>::insert(const ItemType& p)
{
    // An existing active key is updated and is not a new insertion.
    HASH_INDEX_T loc = probe(p.first);
    if(loc != npos && table_[loc] != nullptr)
    {
        table_[loc]->item.second = p.second;
        return;
    }

    // Deleted buckets still count toward the load factor until rehashing.
    double load = static_cast<double>(occupied_) /
                  static_cast<double>(CAPACITIES[mIndex_]);
    if(load >= resizeAlpha_)
    {
        resize();
        loc = probe(p.first);
    }

    if(loc == npos)
    {
        throw std::logic_error("No free hash table location");
    }

    table_[loc] = new HashItem(p);
    ++size_;
    ++occupied_;
}

template<typename K, typename V, typename Prober, typename Hash, typename KEqual>
void HashTable<K,V,Prober,Hash,KEqual>::remove(const KeyType& key)
{
    HashItem* item = internalFind(key);
    if(item != nullptr)
    {
        item->deleted = true;
        --size_;
    }
}

template<typename K, typename V, typename Prober, typename Hash, typename KEqual>
typename HashTable<K,V,Prober,Hash,KEqual>::ItemType const *
HashTable<K,V,Prober,Hash,KEqual>::find(const KeyType& key) const
{
    HashItem* item = internalFind(key);
    return item == nullptr ? nullptr : &item->item;
}

template<typename K, typename V, typename Prober, typename Hash, typename KEqual>
typename HashTable<K,V,Prober,Hash,KEqual>::ItemType *
HashTable<K,V,Prober,Hash,KEqual>::find(const KeyType& key)
{
    HashItem* item = internalFind(key);
    return item == nullptr ? nullptr : &item->item;
}

template<typename K, typename V, typename Prober, typename Hash, typename KEqual>
const typename HashTable<K,V,Prober,Hash,KEqual>::ValueType&
HashTable<K,V,Prober,Hash,KEqual>::at(const KeyType& key) const
{
    HashItem const * item = this->internalFind(key);
    if(item == nullptr) { throw std::out_of_range("Bad key"); }
    return item->item.second;
}

template<typename K, typename V, typename Prober, typename Hash, typename KEqual>
typename HashTable<K,V,Prober,Hash,KEqual>::ValueType&
HashTable<K,V,Prober,Hash,KEqual>::at(const KeyType& key)
{
    HashItem * item = this->internalFind(key);
    if(item == nullptr) { throw std::out_of_range("Bad key"); }
    return item->item.second;
}

template<typename K, typename V, typename Prober, typename Hash, typename KEqual>
const typename HashTable<K,V,Prober,Hash,KEqual>::ValueType&
HashTable<K,V,Prober,Hash,KEqual>::operator[](const KeyType& key) const
{
    return this->at(key);
}

template<typename K, typename V, typename Prober, typename Hash, typename KEqual>
typename HashTable<K,V,Prober,Hash,KEqual>::ValueType&
HashTable<K,V,Prober,Hash,KEqual>::operator[](const KeyType& key)
{
    return this->at(key);
}

template<typename K, typename V, typename Prober, typename Hash, typename KEqual>
typename HashTable<K,V,Prober,Hash,KEqual>::HashItem*
HashTable<K,V,Prober,Hash,KEqual>::internalFind(const KeyType& key) const
{
    HASH_INDEX_T h = this->probe(key);
    if(h == npos || table_[h] == nullptr || table_[h]->deleted)
    {
        return nullptr;
    }
    return table_[h];
}

template<typename K, typename V, typename Prober, typename Hash, typename KEqual>
void HashTable<K,V,Prober,Hash,KEqual>::resize()
{
    const HASH_INDEX_T numCapacities =
        sizeof(CAPACITIES) / sizeof(CAPACITIES[0]);
    if(mIndex_ + 1 >= numCapacities)
    {
        throw std::logic_error("No larger hash table capacity available");
    }

    std::vector<HashItem*> oldTable;
    oldTable.swap(table_);

    ++mIndex_;
    table_.assign(CAPACITIES[mIndex_], nullptr);
    occupied_ = 0;

    for(HASH_INDEX_T i = 0; i < oldTable.size(); ++i)
    {
        HashItem* oldItem = oldTable[i];
        if(oldItem == nullptr)
        {
            continue;
        }

        if(oldItem->deleted)
        {
            delete oldItem;
            continue;
        }

        HASH_INDEX_T loc = probe(oldItem->item.first);
        if(loc == npos)
        {
            delete oldItem;
            throw std::logic_error("Unable to rehash item");
        }
        table_[loc] = oldItem;
        ++occupied_;
    }
}

template<typename K, typename V, typename Prober, typename Hash, typename KEqual>
HASH_INDEX_T HashTable<K,V,Prober,Hash,KEqual>::probe(const KeyType& key) const
{
    HASH_INDEX_T h = hash_(key) % CAPACITIES[mIndex_];
    prober_.init(h, CAPACITIES[mIndex_], key);

    HASH_INDEX_T loc = prober_.next();
    totalProbes_++;
    while(Prober::npos != loc)
    {
        if(nullptr == table_[loc])
        {
            return loc;
        }
        else if(!table_[loc]->deleted && kequal_(table_[loc]->item.first, key))
        {
            return loc;
        }
        loc = prober_.next();
        totalProbes_++;
    }

    return npos;
}

template<typename K, typename V, typename Prober, typename Hash, typename KEqual>
void HashTable<K, V, Prober, Hash, KEqual>::reportAll(std::ostream& out) const
{
    for(HASH_INDEX_T i = 0; i < CAPACITIES[mIndex_]; ++i)
    {
        if(table_[i] != nullptr && !table_[i]->deleted)
        {
            out << "Bucket " << i << ": "
                << table_[i]->item.first << " "
                << table_[i]->item.second << std::endl;
        }
    }
}

#endif

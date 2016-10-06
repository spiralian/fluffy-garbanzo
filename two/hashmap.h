#ifndef HASHMAP_H
#define HASHMAP_H

#include <mutex>
#include <atomic>
#include <vector>
#include <assert.h>
#include <functional>

template <class Mutex>
class Striped
{
    // an array of mutex instances, each mutex will be responsible of protecting a couple of buckets in the hashmap
	Mutex* mutex;
	const int numStripes;

public:

	Striped(const int numStripes_)
		: mutex(new Mutex[numStripes_]), numStripes(numStripes_)
	{
		assert(numStripes > 0);
	}

	~Striped()
	{
        for (int i = 0; i < numStripes; ++i)
            mutex[i].unlock();
		delete[] mutex;
	}

    // get the mutex responsible for the given key
	Mutex& get(const int key) const
	{
		return mutex[key % numStripes];
	}
};

template <typename Key, typename Value>
class HashMap
{
    // linked list holding the key value pairs for the same hash
	struct Bucket
	{
		typedef std::pair<const Key, Value> KeyValuePair;
		KeyValuePair keyValue;
		Bucket* next;
		Bucket(const Key& key, const Value& value) : keyValue(key, value), next(nullptr) {}
	};
    
    // a very simple hash function
	unsigned long getHash(const Key& key) const
	{
		return key % numBuckets;
	}

    // the number of diffrent mutex instances to create
	static const int lockStrip = 8;
	typedef std::lock_guard<std::mutex> ScopedLock;

public:

	HashMap(const int numBuckets_ = lockStrip * 8)
		: numBuckets(numBuckets_),
#ifdef TEST_MODE
        numItems(0),
#endif
		buckets(new Bucket*[numBuckets]()),
		mutex(numBuckets / lockStrip)
    
	{
		assert(numBuckets % lockStrip == 0);
	}

	~HashMap()
	{
		for (int i = 0; i < numBuckets; ++i)
		{
			Bucket* entry = buckets[i];
			while (entry != nullptr)
            {
				Bucket* prev = entry;
				entry = entry->next;
				delete prev;
			}
			buckets[i] = nullptr;
		}
		delete [] buckets;
	}

	void insert(const Key &key, const Value &value)
	{
		unsigned long hash = getHash(key);
		Bucket* prev = nullptr;

		ScopedLock guard(mutex.get(key));
		Bucket* entry = buckets[hash];
        
        // search for entry with key
		while (entry != nullptr && entry->keyValue.first != key)
        {
			prev = entry;
			entry = entry->next;
		}

		if (entry == nullptr)
        {
			entry = new Bucket(key, value);
			if (prev == nullptr)
                // this is the first entry for this bucket
				buckets[hash] = entry;
			else
                // add entry to end of the bucket
                prev->next = entry;
#ifdef TEST_MODE
            ++numItems;
#endif            
		}
        else
            // entry found with key, update with new value
			entry->keyValue.second = value;
	}

	bool get(const Key &key, Value &value)
	{
		unsigned long hash = getHash(key);

		ScopedLock guard(mutex.get(key));
		Bucket* entry = buckets[hash];
        
        // search for entry with key
		while (entry != nullptr)
        {
			if (entry->keyValue.first == key)
            {
                // key found, set value
				value = entry->keyValue.second;
				return true;
			}
			entry = entry->next;
		}
        
        // key not found
		return false;
	}

	void remove(const Key &key)
	{
		unsigned long hash = getHash(key);
		Bucket* prev = nullptr;

		ScopedLock guard(mutex.get(key));
		Bucket* entry = buckets[hash];

        // search for entry with key
		while (entry != nullptr && entry->keyValue.first != key)
        {
			prev = entry;
			entry = entry->next;
		}

		if (entry == nullptr)
            // key not found, nothing to remove
			return;
		else
        {
			if (prev == nullptr)
            {
                // this is the first item in the bucket
				buckets[hash] = entry->next;
			} else
            {
				prev->next = entry->next;
			}
#ifdef TEST_MODE
            --numItems;
#endif
			delete entry;
		}
	}


private:

	const int numBuckets;

	Bucket **buckets;
	Striped<std::mutex> mutex;
    
#ifdef TEST_MODE
    std::atomic<int> numItems;
public:
    int getNumItems() const { return numItems; };
#endif

};

#endif // HASHMAP_H

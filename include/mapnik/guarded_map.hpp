#ifndef MAPNIK_GUARDED_MAP_HPP
#define MAPNIK_GUARDED_MAP_HPP

// stl
#include <memory>
#include <map>
#include <mutex>

namespace mapnik
{

template <class K, class V, class Compare = std::less<K>,
          class Allocator = std::allocator<std::pair<const K, V> > >

class guarded_map
{
  private:
    typedef std::map<K, V, Compare, Allocator> map_type;
    typedef std::lock_guard<std::mutex> lock_guard_type;

    map_type map;
    std::mutex mutex;

  public:
    typedef typename map_type::const_iterator const_iterator;

    bool empty()
    {
      lock_guard_type guard(this->mutex);
      return this->map.empty();
    }

    const_iterator cbegin()
    {
      lock_guard_type guard(this->mutex);
      return this->map.cbegin();
    }

    const_iterator cend()
    {
      lock_guard_type guard(this->mutex);
      return this->map.cend();
    }

    std::pair<const_iterator, bool> emplace(K key, V value)
    {
      lock_guard_type guard(this->mutex);
      return this->map.emplace(key, value);
    }

    const_iterator find(K key)
    {
      lock_guard_type guard(this->mutex);
      return this->map.find(key);
    }

    std::pair<const_iterator, bool> insert(K key, V value)
    {
      lock_guard_type guard(this->mutex);
      return this->map.emplace(key, value);
    }

    size_t size()
    {
      lock_guard_type guard(this->mutex);
      return this->map.size();
    }
};

} //ns mapnik

#endif // MAPNIK_GUARDED_MAP_HPP


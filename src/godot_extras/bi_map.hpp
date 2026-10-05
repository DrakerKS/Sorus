#include "godot_cpp/templates/vector.hpp"
#include "godot_cpp/templates/pair.hpp"
#include "godot_cpp/templates/hash_map.hpp"

#include <cstddef>
#include <type_traits>

namespace godot {
  namespace internal {
    template<typename TKey, typename TValue, bool IsMulti = false>
    class BiMapCore {
      using TContainer = Vector<TValue>;

      using TValueContainer = std::conditional_t<IsMulti, TContainer, TValue>;
      using Entry = Pair<TKey, TValueContainer>;

    private:
      template<typename T>
      static constexpr bool is_key_type = std::is_same_v<T, TKey>;

      template<typename T>
      static constexpr bool is_convertible_to_key = std::is_convertible_v<T, TKey>;

      template<typename T>
      static constexpr bool is_value_type = std::is_same_v<T, TValue>;

      template<typename T>
      static constexpr bool is_convertible_to_value = std::is_convertible_v<T, TValue>;

      template<typename T>
      static constexpr bool is_container = std::is_same_v<T, TContainer>;

      void update_entry_index(int p_from, int p_to) {
        Entry moved = entries[p_from];

        key_to_idx[moved.first] = p_to;

        if constexpr (IsMulti) {
          for (const TValue &value : moved.second) {
            value_to_idx[value] = p_to;
          }
        } else {
          value_to_idx[moved.second] = p_to;
        }

        entries.set(p_to, entries[p_from]);
      }

    protected:
      Vector<Entry> entries {};
      HashMap<TKey, size_t> key_to_idx {};
      HashMap<TValue, size_t> value_to_idx {};

    public:
      bool is_empty() const {
        return entries.is_empty();
      }

      size_t size() const {
        return entries.size();
      }

      template<typename T>
      requires (is_convertible_to_key<T>)
      bool has_key(const T &p_key) const{
        if constexpr (is_key_type<T>) {
          return key_to_idx.has(p_key);
        } else {
          return key_to_idx.has(TKey{p_key});
        }
      }

      template<typename T>
      requires (is_convertible_to_value<T>)
      bool has_value(const T &p_value) const{
        if constexpr (is_value_type<T>) {
          return value_to_idx.has(p_value);
        } else {
          return value_to_idx.has(TValue{p_value});
        }
      }

      template<typename T>
      requires (not std::is_same_v<TKey, TValue> && (is_convertible_to_key<T> || is_convertible_to_value<T>))
      bool has(const T &p_query) const {
        if constexpr (is_convertible_to_key<T>) {
          return has_key(p_query);
        } else {
          return has_value(p_query);
        }
      }

      template<typename T>
      requires (is_convertible_to_key<T>)
      int find_key(const T &p_key) const {
        if (not has_key(p_key)) {
          return -1;
        }

        if constexpr (is_key_type<T>) {
          return key_to_idx[p_key];
        } else {
          return key_to_idx[TKey{p_key}];
        }
      }

      template<typename T>
      requires (is_convertible_to_value<T>)
      int find_value(const T &p_value) const {
        if (not has_value(p_value)) {
          return -1;
        }

        if constexpr (is_value_type<T>) {
          return value_to_idx[p_value];
        } else {
          return value_to_idx[TValue{p_value}];
        }
      }

      template<typename T>
      requires (not std::is_same_v<TKey, TValue> && (is_convertible_to_key<T> || is_convertible_to_value<T>))
      int find(const T &p_query) const {
        if constexpr (is_convertible_to_key<T>) {
          return find_key(p_query);
        } else {
          return find_value(p_query);
        }
      }

      template<typename T>
      requires(is_convertible_to_value<T>)
      const TKey& get_key(const T& p_value) const {
        if constexpr (is_value_type<T>) {
          return entries[value_to_idx[p_value]].first;
        } else {
          return entries[value_to_idx[TValue{p_value}]].first;
        }
      }

      template<typename T>
      requires(is_convertible_to_key<T>)
      const auto& get_value(const T& p_key) const {
        if constexpr (is_key_type<T>) {
          return entries[key_to_idx[p_key]].second;
        } else {
          return entries[key_to_idx[TKey{p_key}]].second;
        }
      }

      template<typename T>
      requires (not std::is_same_v<TKey, TValue> && (is_convertible_to_key<T> || is_convertible_to_value<T>))
      auto get(const T &p_query) const {
        if constexpr (is_convertible_to_key<T>) {
          return get_value(p_query);
        } else {
          return get_key(p_query);
        }
      }

    public:
      template<typename T>
      requires (is_convertible_to_key<T>)
      void erase_key(const T &p_key) {
        int idx = find_key(p_key);
        if (idx == -1) {
          return;
        }

        int last_idx = size() - 1;
        Entry target = entries[idx];

        if (idx != last_idx) {
          entries.set(idx, entries[last_idx]);
          update_entry_index(last_idx, idx);
        }

        key_to_idx.erase(target.first);

        if constexpr (IsMulti) {
          for (const TValue &value : target.second) {
            value_to_idx.erase(value);
          }
        } else {
          value_to_idx.erase(target.second);
        }

        entries.remove_at(last_idx);
      }

      template<typename T>
      requires (is_convertible_to_value<T>)
      void erase_value(const T &p_value) {
        int idx = find_value(p_value);
        if (idx == -1) {
          return;
        }

        Entry target = entries[idx];
        TValue value;
        if constexpr (is_value_type<T>) {
          value = p_value;
        } else {
          value = TValue{p_value};
        }

        value_to_idx.erase(value);

        if constexpr (IsMulti) {
          target.second.remove_at(target.second.find(value));

          if (not target.second.is_empty()) {
            entries.set(idx, target);
            return;
          }
        }

        int last_idx = size() - 1;
        if (idx != last_idx) {
          update_entry_index(last_idx, idx);
        }

        key_to_idx.erase(target.first);
        entries.remove_at(last_idx);
      }

      template<typename T>
      requires (not std::is_same_v<TKey, TValue> && (is_convertible_to_key<T> || is_convertible_to_value<T>))
      void erase(const T &p_query) {
        if constexpr (is_convertible_to_key<T>) {
          erase_key(p_query);
        } else {
          erase_value(p_query);
        }
      }

      template<typename K, typename V>
      requires (is_convertible_to_key<K> && is_convertible_to_value<V>)
      bool insert(const K &p_key, const V &p_input) {
        if (has_value(p_input)) {
          return false;
        }
        
        int idx = find_key(p_key);
        if (idx >= 0 && not IsMulti) {
          return false;
        }

        TKey key;
        if constexpr (is_key_type<K>) {
          key = p_key;
        } else {
          key = TKey{p_key};
        }

        TValue value;
        if constexpr (is_value_type<V>) {
          value = p_input;
        } else {
          value = TValue{p_input};
        }

        if (idx == -1) {
          idx = size();

          if constexpr (IsMulti) {
            entries.append(Entry {key, TValueContainer{value}});
          } else {
            entries.append(Entry {key, value});
          }
        } else if constexpr (IsMulti) {
          Entry entry = entries[idx];
          entry.second.append(value);
          entries.set(idx, entry);
        }
        
        key_to_idx[key] = idx;
        value_to_idx[value] = idx;

        return true;
      }

      template<typename K, typename V>
      requires (IsMulti && is_convertible_to_key<K> && is_container<V>)
      int insert(const K &p_key, const V &p_values) {
        int add_count = 0;

        for (const TValue &value : p_values) {
          add_count += insert(p_key, value);
        }

        return add_count;
      }

      void clear() {
        entries.clear();
        key_to_idx.clear();
        value_to_idx.clear();
      }

      const Vector<Entry> &get_entries() const {
        return entries;
      }

      void show_entries() const requires (std::is_convertible_v<TKey, Variant> && std::is_convertible_v<TValue, Variant>) {
        for (const auto &entry : entries) {
          String entry_value {};
          
          if constexpr (IsMulti) {
            for (const TValue &value : entry.second) {
              if (entry_value.is_empty()) {
                entry_value += "[";
              } else {
                entry_value += ", ";
              }

              entry_value += value;
            }
            entry_value += "]";
          } else {
            const TValue &value = entry.second;
            entry_value += value;
          }
          
          print_line(entry.first, ": ", entry_value);
        }
      }

      BiMapCore() = default;
      ~BiMapCore() = default;
    };
  }

  template <typename TKey, typename TValue>
  using BiMap = internal::BiMapCore<TKey, TValue>;

  template <typename TKey, typename TValue>
  using BiMultimap = internal::BiMapCore<TKey, TValue, true>;
}


#include <queue>
#include <vector>

#include "state.h"

template<typename T,
         typename Sequence = std::vector<T> >
class myQueue : public std::priority_queue<T, Sequence, myHashComparison> {
public:

    bool edit(const T& value) {
        for (auto it = this->c.begin(); it != this->c.end(); ++it) {
            if (*it == value) {
                this->c.erase(it);
                // *it = value;
                // std::make_heap(this->c.begin(), this->c.end(), this->comp);
                this.push(value);
                return true;
            }
        }
        return false;
    }

    bool remove(const T& value) {
        for (auto it = this->c.begin(); it != this->c.end(); ++it) {
            if (*it == value) {
                this->c.erase(it);
                std::make_heap(this->c.begin(), this->c.end(), this->comp);
                return true;
            }
        }
        return false;
    }
};

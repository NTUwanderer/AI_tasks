#include <queue>

template<typename T>
class myQueue : public std::priority_queue<T, std::vector<T>>
{
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
                return true;
            }
        }
        return false;
    }
};

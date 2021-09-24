#ifndef lacpp_sorted_list_hpp
#define lacpp_sorted_list_hpp lacpp_sorted_list_hpp
#include <mutex>
#include <iostream>
#include <atomic>

/* a sorted list implementation by David Klaftenegger, 2015
 * please report bugs or suggest improvements to david.klaftenegger@it.uu.se
 */

/* struct for list nodes */

namespace assignment
{
	class qnode{
		public:
			qnode() = default;
			~qnode() {
			}
			std::atomic<bool> locked = {false};

	};

	class clh_lock{

		private:
			std::atomic<qnode*> tail = {new qnode};
			static thread_local qnode* node;
			static thread_local qnode* pred;

		public:
			clh_lock() = default;
			~clh_lock(){

			}
			void lock(){
				pred = tail.exchange(node);
				while(pred->locked)
				{
					// im gonna spiiiiiiiiiiin
				}
			}
			void unlock(){
				node->locked = false;
				node = pred;
			}

	};
}
template<typename T>
struct node {
	T value;
	node<T>* next;
	std::mutex lock;
};


/* non-concurrent sorted singly-linked list */
template<typename T>
class sorted_list {
	node<T>* first = nullptr;
	assignment::clh_lock head_lock; // need this to check edge case on insert where first == null

	public:
		/* default implementations:
		 * default constructor
		 * copy constructor (note: shallow copy)
		 * move constructor
		 * copy assignment operator (note: shallow copy)
		 * move assignment operator
		 *
		 * The first is required due to the others,
		 * which are explicitly listed due to the rule of five.
		 */
		sorted_list() = default;
		sorted_list(const sorted_list<T>& other) = default;
		sorted_list(sorted_list<T>&& other) = default;
		sorted_list<T>& operator=(const sorted_list<T>& other) = default;
		sorted_list<T>& operator=(sorted_list<T>&& other) = default;
		~sorted_list() {
			while(first != nullptr) {
				remove(first->value);
			}
		}
		/* insert v into the list */
		void insert(T v) {
			head_lock.lock();
			if(first != nullptr)
				first->lock.lock();
			/* first find position */
			node<T>* pred = nullptr;
			node<T>* succ = first;
			while(succ != nullptr && succ->value < v) {
				if (pred != nullptr)
					pred->lock.unlock();
				else{
					head_lock.unlock();
				}
				pred = succ;
				succ = succ->next;
				if(succ != nullptr)
					succ->lock.lock();
			}

			/* construct new node */
			node<T>* current = new node<T>();
			current->value = v;
			current->lock.lock();
			current->next = succ;

			// Will this be the first element in the list?
			if(pred == nullptr) {
				first = current;
				first->lock.unlock();

				//Is this the only element in the list?
				if(first->next != nullptr)
					first->next->lock.unlock();
				head_lock.unlock();
			} else {
				pred->next = current;
				if(succ != nullptr)
				{
					succ->lock.unlock();
				}
				current->lock.unlock();
				pred->lock.unlock();
			}
		}
		void remove(T v) {
			head_lock.lock();

			if(first == nullptr) //empty list
			{
				head_lock.unlock();
				return;
			}
			first->lock.lock();
			/* first find position */
			node<T>* pred = nullptr;
			node<T>* current = first;
			// Traverse the linked list until it ends or a higher value is found
			while(current != nullptr && current->value < v) {
				if (pred != nullptr) 
					pred->lock.unlock();
				else
					head_lock.unlock();
				pred = current;
				current = current->next;
				if(current != nullptr)
					current->lock.lock();
			}
			// Is the current value we stopped at the one we're looking for?
			if(current == nullptr || current->value != v) {
				if(pred != nullptr){
					pred->lock.unlock();
				}
				if(current != nullptr){
					current->lock.unlock();
					if(pred == nullptr) //This covers the case of having a remove fail at the start of the list
						head_lock.unlock();	
				}
				return;
			}
			// Is the element to be removed the first one in the list?
			if(pred == nullptr) {
				first = current->next;
				current->lock.unlock();
				head_lock.unlock();
			} else {
				pred->next = current->next;
				pred->lock.unlock();
				current->lock.unlock();
			}
			delete current;
		}

		/* count elements with value v in the list */
		std::size_t count(T v) {
			// std::cout << "count\n";
			std::size_t cnt = 0;
			/* first go to value v */
			head_lock.lock();
			node<T>* pred = nullptr; // need this to track previous lock
			node<T>* current = first;
			if(current == nullptr) // empty list
			{
				head_lock.unlock();
				return cnt;
			}

			current->lock.lock();
			head_lock.unlock();
			
			while(current != nullptr && current->value < v) {
				if(pred != nullptr) 
					pred->lock.unlock();
				pred = current;
				current = current->next;
				if(current != nullptr)
					current->lock.lock();
			}
			/* count elements */
			while(current != nullptr && current->value == v) {
				cnt++;
				if(pred != nullptr) 
					pred->lock.unlock();
				pred = current;
				current = current->next;
				if(current != nullptr)
					current->lock.lock();
			}
			if(pred != nullptr)
				pred->lock.unlock();

			if(current != nullptr)
				current->lock.unlock();

			return cnt;
		}
};

#endif // lacpp_sorted_list_hpp

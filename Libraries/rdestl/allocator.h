#ifndef RDESTL_ALLOCATOR_H
#define RDESTL_ALLOCATOR_H

namespace rde
{

// CONCEPT!
class allocator
{
public:
	explicit allocator(const char* name = "DEFAULT"):	m_name(name) {}
	explicit allocator(const allocator& other): m_name(other.m_name) {}
	// Copy ctor generated by compiler.
	// allocator(const allocator&)
	~allocator() {}

	// Generated by compiler.
	//allocator& operator=(const allocator&)

	void* allocate(unsigned int bytes, int flags = 0);
	// Not supported for standard allocator for the time being.
	void* allocate_aligned(unsigned int bytes, unsigned int alignment, int flags = 0);
	void deallocate(void* ptr, unsigned int bytes);

	const char* get_name() const;

private:
	const char*	m_name;
};

// True if lhs can free memory allocated by rhs and vice-versa.
inline bool operator==(const allocator& /*lhs*/, const allocator& /*rhs*/)
{
	return true;
}
inline bool operator!=(const allocator& lhs, const allocator& rhs)
{
	return !(lhs == rhs);
}

inline void* allocator::allocate(unsigned int bytes, int)
{
	return operator new(bytes);
}

inline void allocator::deallocate(void* ptr, unsigned int)
{
	operator delete(ptr);
}

} // namespace rde

//-----------------------------------------------------------------------------
#endif // #ifndef RDESTL_ALLOCATOR_H

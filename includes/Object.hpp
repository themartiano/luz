#ifndef OBJECT_HPP
# define OBJECT_HPP

class	Object
{
	public:
		Object(void);

	private:
		void	*_object;
		int		_type;
		void	*_prev;
		void	*_next;

};

#endif
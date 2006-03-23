#ifndef CONFIG_HPP
#define CONFIG_HPP

// Window DLL support

#ifdef _WINDOWS
# define MAPNIK_DECL __declspec (dllexport)
# pragma warning( disable: 4251 )
# pragma warning( disable: 4275 )
# if (_MSC_VER >= 1400) // vc8
#   pragma warning(disable : 4996) //_CRT_SECURE_NO_DEPRECATE
# endif
#else
# define MAPNIK_DECL 
#endif

#endif

#ifndef CONFIG_HPP
#define CONFIG_HPP

// Window DLL support

#ifdef _WINDOWS
# define MAPNIK_DECL __declspec (dllexport)
# pragma warning( disable: 4251 )
# pragma warning( disable: 4275 )
#else
# define MAPNIK_DECL 
#endif

#endif
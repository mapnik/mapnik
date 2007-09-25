#ifndef MAPNIK_CONFIG_ERROR_INCLUDED
#define MAPNIK_CONFIG_ERROR_INCLUDED

#include <iostream>
#include <sstream>

namespace mapnik {

    class config_error : public std::exception
    {
        public:
            config_error() {}

            config_error( const std::string & what ) :
                what_( what )
            {
            }
            virtual ~config_error() throw() {};

            virtual const char * what() const throw()
            {
                std::ostringstream os;
                os << what_;
                if ( ! context_.empty() )
                {
                    os << std::endl << context_;
                }
                os << ".";
                return os.str().c_str();    
            }

            void append_context(const std::string & ctx) const
            {
                if ( ! context_.empty() )
                {
                    context_ += " ";
                }
                context_ += ctx;
            }

        protected:
            std::string what_;
            mutable std::string context_;
    };
}

#endif // MAPNIK_CONFIG_ERROR_INCLUDED

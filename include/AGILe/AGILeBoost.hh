#ifndef AGILE_AGILEBOOST_HH
#define AGILE_AGILEBOOST_HH

#include "boost/smart_ptr.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/assign.hpp"

#include <boost/algorithm/string.hpp>

#ifndef foreach
#include <boost/foreach.hpp>
namespace boost {
  // Suggested work-around for https://svn.boost.org/trac/boost/ticket/6131
  namespace BOOST_FOREACH = foreach;
}
#define foreach BOOST_FOREACH
#endif

namespace AGILe {
  using boost::shared_ptr;
  using boost::lexical_cast;
  using namespace boost::assign;
}

#endif

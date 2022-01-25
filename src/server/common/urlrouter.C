#ifndef COMMON_URLROUTER
#define COMMON_URLROUTER

#include <Wt/WObject.h>
#include <Wt/WApplication.h>

#include <boost/regex.hpp>

#include <functional>
#include <string>
#include <vector>
#include <utility>
#include <stdexcept>
#include <sstream>

namespace Wt {
  class WApplication;
}

// Needed because some Wt include file brings in boost::placeholders clash.
namespace sph = std::placeholders;

/**
Exception thrown in case of error by UrlRouter operations.

 */
class UrlRouterError : public std::runtime_error
{
public:
    explicit UrlRouterError(const std::string& what)
        : std::runtime_error(what)
    { }
};

/**
A regex-based URL router/dispatcher for Wt inspired by the smart [Django URLconf]
(https://docs.djangoproject.com/en/dev/topics/http/urls).

It reacts to the internalPathChanged() signal of the Wt::WApplication set as its parent
and routes to the **first** matching handler, so the **order** of the calls to add() is
important.

The regex string must be a valid regex from boost::regex.

Since its ownership is transferred to the current WApplication, it must be created on the
heap.

Usage example:

~~~
class View // probably inherit from a Wt view class
{
    void specialcase2003();
    void yeararchive(const std::string& arg1);
    void montharchive(const std::string& arg1, const std::string& arg2);
    void articledetail(const std::string& arg1, const std::string& arg2,
                       const std::string& arg3);

    UrlRouter<Mock>* _r;

View()
{
...
     _r = new UrlRouter<View>(Wt::WApplication::instance(), this);
     _r->add("/articles/2003/",                     &View::specialcase2003);
     _r->add("/articles/(\\d{4})/",                 &View::yeararchive);
     _r->add("/articles/(\\d{4})/(\\d{2})/",        &View::montharchive);
     _r->add("/articles/(\\d{4})/(\\d{2})/(\\d+)/", &View::articledetail);
...
}
~~~

To capture a value from the URL, put a grouping parenthesis around it. A request to
`/articles/2005/03/` would match the third entry in the list. The UrlRouter will call
`this->montharchive('2005', '03')`. Arguments are always captured as strings; it is up to
the handler to do proper conversion if needed.

Compared to the Python case, the patterns are uglier due to the fact that C++ strings
require to escape all backslashes. An alternative is to use C++11 raw literals, thus
avoiding escaping backslashes at the cost of some confusion because they require an
additional set of parenthesis that are _not_ part of the regex:

~~~
r->add(R"(/articles/2003/)",                  &View::specialcase2003);
r->add(R"(/articles/(\d{4})/)",               &View::yeararchive);
r->add(R"(/articles/(\d{4})/(\d{2})/)",       &View::montharchive);
r->add(R"(/articles/(\d{4})/(\d{2})/(\d+)/)", &View::articledetail);
~~~

In case of error in the usage of the API, it will throw an UrlRouterError exception.

Error Handling
TODO: default handler handler404? Add registration of error handlers?

See also the UT for additional usage examples.
 */
template<typename H>
class UrlRouter : public Wt::WObject
{
public:
    /**
     * Construct an UrlRouter object. It must be created on the heap.
     *
     * @param parent  The current WApplication, to which ownership will be
     *                transferred. Must be non-null.
     * @param handler The handler object, normally a Wt view.
     *
     * @throw UrlRouterError if any of `parent` or `handler` is null.
     */
    UrlRouter(Wt::WApplication* parent, H* handler) : _handler(handler)
    {
      if (!parent)  { throw UrlRouterError("NULL parent"); }
      if (!handler) { throw UrlRouterError("NULL handler"); }

      parent->internalPathChanged()
              .connect(this, &UrlRouter::onInternalPathChange);
    }

    ~UrlRouter() {}

    /**
     * Add method `pm` of the object `handler` (passed to the costructor)
     * to be called if the incoming ULR matches `pattern`, where `pattern` has 0
     * matching groups.
     */
    void add(const std::string& pattern, void (H::* pm)())
    {
      boost::regex regex(pattern);
      check_groups_args(regex.mark_count(), 0);
      _handlers0.push_back(std::bind(pm, _handler));
      _regex2handler.push_back(std::make_pair(regex, _handlers0.size()-1));
    }

    /**
     * Add method `pm` of the object `handler` (passed to the costructor)
     * to be called if the incoming ULR matches `pattern`, where `pattern` has 1
     * matching group. The matching groups will be passed as arguments of `pm`.
     */
    void add(const std::string& pattern, void (H::* pm)(const std::string& arg1))
    {
      boost::regex regex(pattern);
      check_groups_args(regex.mark_count(), 1);
      _handlers1.push_back(std::bind(pm, _handler, sph::_1));
      _regex2handler.push_back(std::make_pair(regex, _handlers1.size()-1));
    }

    /**
     * Add method `pm` of the object `handler` (passed to the costructor)
     * to be called if the incoming ULR matches `pattern`, where `pattern` has 2
     * matching groups. The matching groups will be passed as arguments of `pm`.
     */
    void add(const std::string& pattern, void (H::* pm)(const std::string& arg1, const std::string& arg2))
    {
      boost::regex regex(pattern);
      check_groups_args(regex.mark_count(), 2);
      _handlers2.push_back(std::bind(pm, _handler, sph::_1, sph::_2));
      _regex2handler.push_back(std::make_pair(regex, _handlers2.size()-1));
    }

    /**
     * Add method `pm` of the object `handler` (passed to the costructor)
     * to be called if the incoming ULR matches `pattern`, where `pattern` has 3
     * matching groups. The matching groups will be passed as arguments of `pm`.
     */
    void add(const std::string& pattern, void (H::* pm)(const std::string& arg1, const std::string& arg2, const std::string& arg3))
    {
      boost::regex regex(pattern);
      check_groups_args(regex.mark_count(), 3);
      _handlers3.push_back(std::bind(pm, _handler, sph::_1, sph::_2, sph::_3));
      _regex2handler.push_back(std::make_pair(regex, _handlers3.size()-1));
    }


private:
    // Make it uncopiable.
    UrlRouter(const UrlRouter&);
    UrlRouter& operator=(const UrlRouter&);

    void onInternalPathChange(const std::string& path)
    {
        /*
         * Iterate through the regexes in the same order as they have been added
         * with add(). First match wins.
         */
        for (std::pair<boost::regex, int>& p : _regex2handler) {
            boost::regex& re = p.first;
            int index = p.second;
            boost::smatch sm;
            if (boost::regex_match(path, sm, re)) {

                // Ugly, repetitive code because I don't know enough C++-fu...

                const int nArgs = re.mark_count() - 1;
                if (nArgs == 0) {
                    handler0_t f = _handlers0[index];
                    f();
                } else if (nArgs == 1) {
                    handler1_t f = _handlers1[index];
                    f(sm[1].str());
                } else if (nArgs == 2) {
                    handler2_t f = _handlers2[index];
                    f(sm[1].str(), sm[2].str());
                } else if (nArgs == 3) {
                    handler3_t f = _handlers3[index];
                    f(sm[1].str(), sm[2].str(), sm[3].str());
                } else {
                    assert(0 && "nArgs too big");
                }
                break;
            }
        }
    }

    void check_groups_args(int groups, int args)
    {
        groups -= 1;
        if (groups != args) {
            _oss << "regex groups: " << groups << " but args: " << args;
            throw UrlRouterError(_oss.str());
        }
    }


    // Map a regex to the index in the _handlersN vector to find the
    // corresponding handler.
    std::vector<std::pair<boost::regex, int> > _regex2handler;

    typedef std::function<void ()>                        handler0_t;
    typedef std::function<void (const std::string& arg1)> handler1_t;
    typedef std::function<void (const std::string& arg1,
                                const std::string& arg2)> handler2_t;
    typedef std::function<void (const std::string& arg1,
                                const std::string& arg2,
                                const std::string& arg3)> handler3_t;

    std::vector<handler0_t> _handlers0;
    std::vector<handler1_t> _handlers1;
    std::vector<handler2_t> _handlers2;
    std::vector<handler3_t> _handlers3;

    H* _handler;

    std::ostringstream _oss;
};

#endif // COMMON_URLROUTER

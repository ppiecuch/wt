/**
 * Translates into an HTML div object.
 */

#ifndef DIV
#define DIV

#include <Wt/WContainerWidget.h>

#include <string>

class Div : public Wt::WContainerWidget
{
public:
    Div(const std::string &id, const std::string &cssClass) {setId(id); setStyleClass(cssClass); }
    explicit Div(const std::string& id)  { setId(id); }
    Div() { }
    virtual ~Div() { }
};


#endif /* DIV */

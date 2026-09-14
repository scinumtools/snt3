#include "snt/c.h"
#include <snt/puq/calc/calculator.h>
#include <snt/puq/quantity.h>
#include <snt/dip/dip.h>
#include <cstring>
#include <stdexcept>
struct snt_quantity { snt::puq::Quantity value; explicit snt_quantity(snt::puq::Quantity q):value(std::move(q)){} };
struct snt_dip { snt::dip::DIP parser; snt::dip::Environment env; bool parsed=false; };
namespace { int fail(snt_error* e,const std::exception& x){static thread_local std::string m;m=x.what();if(e){e->code=1;e->message=m.c_str();}return 1;} void ok(snt_error*e){if(e){e->code=0;e->message=nullptr;}} }
extern "C" int snt_quantity_eval(const char* x,snt_quantity**o,snt_error*e){try{if(!x||!o)throw std::invalid_argument("expression and output are required");*o=new snt_quantity(snt::puq::Calculator().eval(x).value);ok(e);return 0;}catch(const std::exception&x){return fail(e,x);}}
extern "C" int snt_quantity_convert(const snt_quantity*q,const char*u,snt_quantity**o,snt_error*e){try{if(!q||!u||!o)throw std::invalid_argument("quantity, units, and output are required");*o=new snt_quantity(q->value.convert(u));ok(e);return 0;}catch(const std::exception&x){return fail(e,x);}}
extern "C" int snt_quantity_format(const snt_quantity*q,char*b,size_t n,snt_error*e){try{if(!q||!b||!n)throw std::invalid_argument("quantity, buffer, and capacity are required");auto s=q->value.to_string();if(s.size()+1>n)throw std::invalid_argument("output buffer is too small");std::memcpy(b,s.c_str(),s.size()+1);ok(e);return 0;}catch(const std::exception&x){return fail(e,x);}}
extern "C" void snt_quantity_free(snt_quantity*q){delete q;}
extern "C" int snt_dip_create(snt_dip**o,snt_error*e){try{if(!o)throw std::invalid_argument("output is required");*o=new snt_dip;ok(e);return 0;}catch(const std::exception&x){return fail(e,x);}}
extern "C" int snt_dip_add_string(snt_dip*d,const char*x,snt_error*e){try{if(!d||!x)throw std::invalid_argument("DIP and source text are required");d->parser.add_string(x);ok(e);return 0;}catch(const std::exception&x){return fail(e,x);}}
extern "C" int snt_dip_add_file(snt_dip*d,const char*x,snt_error*e){try{if(!d||!x)throw std::invalid_argument("DIP and filename are required");d->parser.add_file(x);ok(e);return 0;}catch(const std::exception&x){return fail(e,x);}}
extern "C" int snt_dip_parse(snt_dip*d,snt_error*e){try{if(!d)throw std::invalid_argument("DIP is required");d->env=d->parser.parse();d->parsed=true;ok(e);return 0;}catch(const std::exception&x){return fail(e,x);}}
extern "C" int snt_dip_get(const snt_dip*d,const char*p,char*b,size_t n,snt_error*e){try{if(!d||!d->parsed||!p||!b||!n)throw std::invalid_argument("parsed DIP, path, buffer, and capacity are required");auto v=d->env.request_node_data(std::string("?")+p);if(!v.value)throw std::runtime_error("DIPL path has no value");auto s=v.value->to_string();if(s.size()+1>n)throw std::invalid_argument("output buffer is too small");std::memcpy(b,s.c_str(),s.size()+1);ok(e);return 0;}catch(const std::exception&x){return fail(e,x);}}
extern "C" void snt_dip_free(snt_dip*d){delete d;}

#include "utilities/growableArray.hpp"
#include "gc_interface/methodInfo.hpp"

MethodInfoAccessList::MethodInfoAccessList() {
  _access_list = new (ResourceObj::C_HEAP, mtInternal) GrowableArray<const Symbol*>(10, true);
}

MethodInfoAccessList::~MethodInfoAccessList() {
  delete _access_list;
}

void MethodInfoAccessList::print_helper(const Symbol* sym) const {
  tty->print("\t%s\n", sym->as_utf8());
}

void MethodInfoAccessList::addAccess(const ciSymbol* access) {
  // Check to make sure this access is unique.
  const Symbol* sym = MethodInfoManager::resolve_ciSymbol(access);
  if (_access_list->find(sym) == -1) {
    _access_list->push(sym);
  }
}

void MethodInfoAccessList::doAccesses(void (*f)(const Symbol*)) const {
  // Iterate across all access fields calling function "f" on each.
  for (int i = 0; i < _access_list->length(); i++) {
    (*f)(_access_list->at(i));
  }
}

void MethodInfoAccessList::print() const {
  for (int i = 0; i < _access_list->length(); i++) {
    tty->print("\t%s\n", _access_list->at(i)->as_utf8());
  }
}

MethodInfo::MethodInfo(const ciSymbol* klass, const ciSymbol* method, const ciSymbol* signature, const MethodInfoAccessList* access_list)
  : _klass(MethodInfoManager::resolve_ciSymbol(klass)),
    _method(MethodInfoManager::resolve_ciSymbol(method)),
    _signature(MethodInfoManager::resolve_ciSymbol(signature)),
    _access_list(access_list)
{}

MethodInfo::~MethodInfo() {
  delete _access_list;
}

const Method* MethodInfo::getMethodReal() const {
  // TODO: Implement me if needed.
  return NULL;
}

const void MethodInfo::print() const {
  if (getKlass())
    tty->print("%s.", getKlass()->as_utf8());
  else
    tty->print("<Unknown Klass>.");

  if (getMethod())
    tty->print("%s", getMethod()->as_utf8());
  else
    tty->print("<Unknown Method>");

  if (getSignature())
    tty->print("%s", getSignature()->as_utf8());
}

const void MethodInfo::print2() const {
  tty->print("MethodInfo for ");
  print();
  tty->print_cr(":");
  _access_list->print();
  tty->cr();
}

GrowableArray<const MethodInfo*>* MethodInfoManager::_infos = new (ResourceObj::C_HEAP, mtInternal) GrowableArray<const MethodInfo*>(512, true);

const MethodInfo* MethodInfoManager::make(const ciSymbol* klass, const ciSymbol* method, const ciSymbol* signature, const MethodInfoAccessList* access_list) {
  assert(access_list != NULL, "Access list cannot be NULL.");

  // Check if a MethodInfo exists with these parameters
  for (int i = 0; i < _infos->length(); i++) {
    const MethodInfo* tmp = _infos->at(i);

    if (tmp->getKlass()->equals(MethodInfoManager::resolve_ciSymbol(klass)->as_utf8()) &&
        tmp->getMethod()->equals(MethodInfoManager::resolve_ciSymbol(method)->as_utf8()) &&
        tmp->getSignature()->equals(MethodInfoManager::resolve_ciSymbol(signature)->as_utf8())) {
      return NULL;
    }
  }
  // This MethodInfo is unique so go ahead and add it
  const MethodInfo* ret = new MethodInfo(klass, method, signature, access_list);
  _infos->append(ret);
  return ret;
}

const MethodInfo* MethodInfoManager::getMethodInfo(const ciSymbol* klass, const ciSymbol* method, const ciSymbol* signature) {
  for (int i = 0; i < _infos->length(); i++) {
    const MethodInfo* tmp = _infos->at(i);

    if (tmp->getKlass()->equals(MethodInfoManager::resolve_ciSymbol(klass)->as_utf8()) &&
        tmp->getMethod()->equals(MethodInfoManager::resolve_ciSymbol(method)->as_utf8()) &&
        tmp->getSignature()->equals(MethodInfoManager::resolve_ciSymbol(signature)->as_utf8())) {
      return tmp;
    }
  }

  return NULL;
}

const MethodInfo* MethodInfoManager::getMethodInfo(const Symbol* klass, const Symbol* method, const Symbol* signature) {
  for (int i = 0; i < _infos->length(); i++) {
    const MethodInfo* tmp = _infos->at(i);

    if (tmp->getKlass()->equals(klass->as_utf8()) && tmp->getMethod()->equals(method->as_utf8()) && tmp->getSignature()->equals(signature->as_utf8())) {
      return tmp;
    }
  }

  return NULL;
}

const Symbol* MethodInfoManager::resolve_ciSymbol(const ciSymbol* sym) {
  return sym->get_symbol();
}

void MethodInfoManager::printAll() {
  tty->print_cr("---------------------------------");
  for (int i = 0; i < _infos->length(); i++) {
    _infos->at(i)->print2();
  }
  tty->print_cr("---------------------------------");
}

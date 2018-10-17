#ifndef YAGE_MACRO_DEFINES_H
#define YAGE_MACRO_DEFINES_H

#define YAGE_CONCATENATE_DETAIL(x, y) x##y
#define YAGE_CONCATENATE(x, y) YAGE_CONCATENATE_DETAIL(x, y)
#define YAGE_MAKE_UNIQUE(x) YAGE_CONCATENATE(x, __COUNTER__)

#define DECLARE_YAGE_CLASS (class_name) \
    friend class Meta::RegisterClass<class_name>;

#endif // !YAGE_MACRO_DEFINES_H

#pragma once
#ifndef SIREN_Pybind11Trampoline_H
#define SIREN_Pybind11Trampoline_H

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/utility.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pybind11/embed.h>

#define SELF_OVERRIDE_PURE(selfname, BaseType, returnType, cfuncname, pyfuncname, ...) \
        const BaseType * ref; \
        if(selfname) { \
            ref = selfname.cast<BaseType *>(); \
        } else { \
            ref = this; \
        } \
        do { \
            do { \
                pybind11::gil_scoped_acquire gil; \
                pybind11::function override \
                    = pybind11::get_override(static_cast<const BaseType *>(ref), pyfuncname); \
                if (override) { \
                    auto o = override(__VA_ARGS__); \
                    if (pybind11::detail::cast_is_temporary_value_reference<returnType>::value) { \
                        static pybind11::detail::override_caster_t<returnType> caster; \
                        return pybind11::detail::cast_ref<returnType>(std::move(o), caster); \
                    } \
                    return pybind11::detail::cast_safe<returnType>(std::move(o)); \
                } \
            } while (false); \
            pybind11::pybind11_fail( \
                "Tried to call pure virtual function \"" PYBIND11_STRINGIFY(BaseType) "::" #cfuncname "\""); \
        } while (false);

#define SELF_OVERRIDE(selfname, BaseType, returnType, cfuncname, pyfuncname, ...) \
        const BaseType * ref; \
        if(selfname) { \
            ref = selfname.cast<BaseType *>(); \
        } else { \
            ref = this; \
        } \
        do { \
            do { \
                pybind11::gil_scoped_acquire gil; \
                pybind11::function override \
                    = pybind11::get_override(static_cast<const BaseType *>(ref), pyfuncname); \
                if (override) { \
                    auto o = override(__VA_ARGS__); \
                    if (pybind11::detail::cast_is_temporary_value_reference<returnType>::value) { \
                        static pybind11::detail::override_caster_t<returnType> caster; \
                        return pybind11::detail::cast_ref<returnType>(std::move(o), caster); \
                    } \
                    return pybind11::detail::cast_safe<returnType>(std::move(o)); \
                } \
            } while (false); \
            return BaseType::cfuncname(__VA_ARGS__); \
        } while (false);


template<typename BaseType, typename TrampolineType>
class Pybind11Trampoline {
    

    // one virtual function needed to call dynaimc_cast below
    virtual void dummy() {};

    public:

    pybind11::object self;

    // First attempts to call a python-side override of the "get_representation" function
    // The assumption is that "get_representation" returns a python dictionary that contains the representation of the object
    // If "get_representation" is not overriden on the python side, then this function returns the contents of __dict__
    pybind11::object get_representation() {
        // First check if "self" is a valid reference to the python side of this object
        // Otherwise use "this" and search for the corresponding python side
        const BaseType * ref;
        if(self) {
            ref = self.cast<BaseType *>();
        } else {
            ref = dynamic_cast<const BaseType *>(this);
            if (!ref) {
                throw std::runtime_error("Cannot cast this to BaseType");
        }
        }

        auto *tinfo = pybind11::detail::get_type_info(typeid(BaseType));
        pybind11::function override_func =
            tinfo ? pybind11::detail::get_type_override(static_cast<const BaseType *>(ref), tinfo, "get_representation") : pybind11::function();
        if (override_func) {
            pybind11::object o = override_func();
            if(not pybind11::isinstance<pybind11::dict>(o)) {
                throw std::runtime_error("get_representation must return a dict");
            }
            return o;
        }

        pybind11::object _self;
        if(this->self) {
            _self = pybind11::reinterpret_borrow<pybind11::object>(this->self);
        } else {
            auto *tinfo = pybind11::detail::get_type_info(typeid(BaseType));
            pybind11::handle self_handle = get_object_handle(dynamic_cast<const BaseType *>(this), tinfo);
            _self = pybind11::reinterpret_borrow<pybind11::object>(self_handle);
        }
        pybind11::dict d;
        if (pybind11::hasattr(_self, "__dict__")) {
            d = _self.attr("__dict__");
        }
        return d;
    }

    static pybind11::tuple pickle_save(BaseType & cpp_obj) {
        pybind11::object x = static_cast<TrampolineType&>(cpp_obj).Pybind11Trampoline<BaseType, TrampolineType>::get_representation();
        return pybind11::make_tuple(x);
    }

    static std::pair<std::unique_ptr<BaseType>, pybind11::dict> pickle_load(const pybind11::tuple &t) {
        if (t.size() != 1) {
            throw std::runtime_error("Invalid state!");
        }
        auto cpp_state = std::unique_ptr<BaseType>(new TrampolineType);
        auto py_state = t[0].cast<pybind11::dict>();
        return std::make_pair(std::move(cpp_state), py_state);
    }

    template<typename Archive>
    void save(Archive & archive, std::uint32_t const version) const {
        if(version == 0) {

            // Either use *self* or find the corresponsing python object for the instance of this class
            // Pass that python object (self) to pickle to get the byestream
            pybind11::object obj;
            if(this->self) {
                obj = this->self;
            } else {
                auto *tinfo = pybind11::detail::get_type_info(typeid(TrampolineType));
                pybind11::handle self_handle = get_object_handle(dynamic_cast<const TrampolineType *>(this), tinfo);
                obj = pybind11::reinterpret_borrow<pybind11::object>(self_handle);
            }

			pybind11::module pkl = pybind11::module::import("pickle");
			pybind11::bytes bytes = pkl.attr("dumps")(obj);
			std::string str_repr = (std::string)(bytes.attr("hex")().cast<std::string>());

			archive(::cereal::make_nvp("PythonPickleBytesRepresentation", str_repr));

            archive(cereal::virtual_base_class<BaseType>(dynamic_cast<const TrampolineType*>(this)));

        } else {
            throw std::runtime_error("BaseType only supports version <= 0!");
        }
    }

    template<typename Archive>
    void load(Archive & archive, std::uint32_t version) {
        if(version == 0) {
            

            std::string str_repr;
			archive(::cereal::make_nvp("PythonPickleBytesRepresentation", str_repr));

            pybind11::module pkl = pybind11::module::import("pickle");

            pybind11::object bytes_module = pybind11::module::import("builtins").attr("bytes");
            pybind11::object bytes = bytes_module.attr("fromhex")(str_repr);
            
            pkl.attr("loads")(bytes);
            this->self = pkl.attr("loads")(bytes);

            archive(cereal::virtual_base_class<BaseType>(dynamic_cast<const TrampolineType*>(this)));

        } else {
            throw std::runtime_error("BaseType only supports version <= 0!");
        }
    }
};

#define RegisterTrampolinePickleMethods(object, TrampolineType) object.def(pybind11::pickle(&TrampolineType::pickle_save, &TrampolineType::pickle_load));
#define TrampolinePickleMethods(TrampolineType) .def(pybind11::pickle(&TrampolineType::pickle_save, &TrampolineType::pickle_load))

#define RegisterTrampolineCerealMethods(BaseType, TrampolineType, Pybind11TrampolineType) \
    CEREAL_CLASS_VERSION(TrampolineType,0); \
    CEREAL_REGISTER_TYPE(TrampolineType); \
    CEREAL_REGISTER_POLYMORPHIC_RELATION(BaseType, TrampolineType); \
    CEREAL_CLASS_VERSION(Pybind11TrampolineType, 0); \
    CEREAL_REGISTER_TYPE(Pybind11TrampolineType); \
    CEREAL_REGISTER_POLYMORPHIC_RELATION(Pybind11TrampolineType, TrampolineType); \

#endif // SIREN_Pybind11Trampoline_H
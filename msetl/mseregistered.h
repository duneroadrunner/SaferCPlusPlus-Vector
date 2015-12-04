#ifndef MSEREGISTERED_H_
#define MSEREGISTERED_H_

#include "mseprimitives.h"
#include <unordered_map>

namespace mse {

	class CSPManager {
	public:
		CSPManager() {}
		~CSPManager() {}
		bool registerPointer(const CSaferPtrBase& sp_ref, void *obj_ptr);
		bool unregisterPointer(const CSaferPtrBase& sp_ref, void *obj_ptr);
		void onObjectDestruction(void *obj_ptr);
		std::unordered_multimap<void*, const CSaferPtrBase&> m_obj_pointer_map;
	};

	extern CSPManager gSPManager;

	/* TRegisteredPointer behaves similar to (and is largely compatible with) native pointers. It inherits the safety features of
	TSaferPtr (default nullptr initialization and check for null pointer dereference). In addition, when pointed at a
	TRegisteredObj, it will be checked for attempted access after destruction. It's essentially intended to be a safe pointer like
	std::shared_ptr, but that does not take ownership of the target object (i.e. does not take responsibility for deallocation).
	Because it does not take ownership, unlike std::shared_ptr, TRegisteredPointer can be used to point to objects on the stack. */
	template<typename _Ty>
	class TRegisteredPointer : public TSaferPtr<_Ty> {
	public:
		TRegisteredPointer(_Ty* ptr = nullptr) : TSaferPtr<_Ty>(ptr) { gSPManager.registerPointer((*this), ptr); }
		TRegisteredPointer(const TRegisteredPointer& src_cref) : TSaferPtr<_Ty>(src_cref.m_ptr) { gSPManager.registerPointer((*this), src_cref.m_ptr); }
		virtual ~TRegisteredPointer() {
			gSPManager.unregisterPointer((*this), (*this).m_ptr);
			gSPManager.onObjectDestruction(this); /* Just in case there's a pointer to this pointer out there. */
		}
		TRegisteredPointer<_Ty>& operator=(_Ty* ptr) {
			gSPManager.unregisterPointer((*this), (*this).m_ptr);
			TSaferPtr<_Ty>::operator=(ptr);
			gSPManager.registerPointer((*this), ptr);
			return (*this);
		}
		TRegisteredPointer<_Ty>& operator=(const TRegisteredPointer<_Ty>& _Right_cref) {
			return operator=(_Right_cref.m_ptr);
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const {
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
			return (*this).m_ptr;
		}
	};

	/* TRegisteredPointerForLegacy is similar to TRegisteredPointer, but more readily converts to a native pointer implicitly. So
	when replacing native pointers with "registered" pointers in legacy code, fewer code changes (explicit casts) may be required
	when using this template. */
	template<typename _Ty>
	class TRegisteredPointerForLegacy : public TSaferPtrForLegacy<_Ty> {
	public:
		TRegisteredPointerForLegacy(_Ty* ptr = nullptr) : TSaferPtrForLegacy<_Ty>(ptr) { gSPManager.registerPointer((*this), ptr); }
		TRegisteredPointerForLegacy(const TRegisteredPointerForLegacy& src_cref) : TSaferPtrForLegacy<_Ty>(src_cref.m_ptr) { gSPManager.registerPointer((*this), src_cref.m_ptr); }
		virtual ~TRegisteredPointerForLegacy() {
			gSPManager.unregisterPointer((*this), (*this).m_ptr);
			gSPManager.onObjectDestruction(this); /* Just in case there's a pointer to this pointer out there. */
		}
		TRegisteredPointerForLegacy<_Ty>& operator=(_Ty* ptr) {
			gSPManager.unregisterPointer((*this), (*this).m_ptr);
			TSaferPtrForLegacy<_Ty>::operator=(ptr);
			gSPManager.registerPointer((*this), ptr);
			return (*this);
		}
		TRegisteredPointerForLegacy<_Ty>& operator=(const TRegisteredPointerForLegacy<_Ty>& _Right_cref) {
			return operator=(_Right_cref.m_ptr);
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		operator _Ty*() const {
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
			return (*this).m_ptr;
		}
	};

	/* TRegisteredObj is intended as a transparent wrapper for other classes/objects. The purpose is to register the object's
	destruction so that TRegisteredPointers will avoid referencing destroyed objects. Note that TRegisteredObj can be used with
	objects allocated on the stack. */
	template<typename _Ty>
	class TRegisteredObj : public _Ty {
	public:
		//using _Ty::_Ty;
		// the version of the compiler (msvc 2013) being used does not yet support inherited constructors, so we use this macro hack
		// for now
		MSE_USING(TRegisteredObj, _Ty);
		virtual ~TRegisteredObj() {
			gSPManager.onObjectDestruction(this);
		}
		TRegisteredPointer<_Ty> operator&() {
			return TRegisteredPointer<_Ty>(this);
		}
		TRegisteredPointer<const _Ty> operator&() const {
			return TRegisteredPointer<const _Ty>(this);
		}
	};

	/* registered_new is intended to be analogous to std:make_shared */
	template <class _Ty, class... Args>
	TRegisteredPointer<_Ty> registered_new(Args&&... args) {
		TRegisteredPointer<_Ty> retval = new TRegisteredObj<_Ty>(args...);
		return retval;
	}
	template <class _Ty>
	void registered_delete(const TRegisteredPointer<_Ty>& regPtrRef) {
		auto a = dynamic_cast<TRegisteredObj<_Ty> *>((_Ty *)regPtrRef);
		delete a;
	}

	template <class _Ty, class... Args>
	TRegisteredPointerForLegacy<_Ty> registered_new_for_legacy(Args&&... args) {
		TRegisteredPointerForLegacy<_Ty> retval = new TRegisteredObj<_Ty>(args...);
		return retval;
	}
	template <class _Ty>
	void registered_delete_for_legacy(const TRegisteredPointerForLegacy<_Ty>& regPtrRef) {
		auto a = dynamic_cast<TRegisteredPointerForLegacy<_Ty> *>((_Ty *)regPtrRef);
		delete a;
	}
}


#endif // MSEREGISTERED_H_

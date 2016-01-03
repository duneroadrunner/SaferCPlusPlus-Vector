#ifndef MSEREGISTERED_H_
#define MSEREGISTERED_H_

#include "mseprimitives.h"
#include <unordered_map>
#include <unordered_set>

namespace mse {

	class CRPManager {
	public:
		CRPManager() {}
		~CRPManager() {}
		bool registerPointer(const CSaferPtrBase& sp_ref) {
			std::unordered_set<const CSaferPtrBase*>::value_type item(&sp_ref);
			auto res = m_ptr_to_regptr_set.insert(item);
			return res.second;
		}
		bool unregisterPointer(const CSaferPtrBase& sp_ref) {
			auto res = m_ptr_to_regptr_set.erase(&sp_ref);
			return (0 != res);
		}
		void onObjectDestruction() {
			for (auto sp_ref_ptr : m_ptr_to_regptr_set) {
				(*sp_ref_ptr).setToNull();
			}
		}
		std::unordered_set<const CSaferPtrBase*> m_ptr_to_regptr_set;
	};

	template<typename _Ty>
	class TRegisteredObj;

	/* TRegisteredPointer behaves similar to (and is largely compatible with) native pointers. It inherits the safety features of
	TSaferPtr (default nullptr initialization and check for null pointer dereference). In addition, when pointed at a
	TRegisteredObj, it will be checked for attempted access after destruction. It's essentially intended to be a safe pointer like
	std::shared_ptr, but that does not take ownership of the target object (i.e. does not take responsibility for deallocation).
	Because it does not take ownership, unlike std::shared_ptr, TRegisteredPointer can be used to point to objects on the stack. */
	template<typename _Ty>
	class TRegisteredPointer : public TSaferPtr<TRegisteredObj<_Ty>> {
	public:
		TRegisteredPointer();
		TRegisteredPointer(TRegisteredObj<_Ty>* ptr);
		TRegisteredPointer(const TRegisteredPointer& src_cref);
		virtual ~TRegisteredPointer();
		TRegisteredPointer<_Ty>& operator=(TRegisteredObj<_Ty>* ptr);
		TRegisteredPointer<_Ty>& operator=(const TRegisteredPointer<_Ty>& _Right_cref);
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const;
		CRPManager& mseRPManager() { return m_mseRPManager; }

		mutable CRPManager m_mseRPManager;
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
			mseRPManager().onObjectDestruction();
		}
		TRegisteredPointer<_Ty> operator&() {
			return TRegisteredPointer<_Ty>(this);
		}
		TRegisteredPointer<const _Ty> operator&() const {
			return TRegisteredPointer<const _Ty>(this);
		}
		CRPManager& mseRPManager() { return m_mseRPManager; }

		mutable CRPManager m_mseRPManager;
	};

	template<typename _Ty>
	TRegisteredPointer<_Ty>::TRegisteredPointer() : TSaferPtr<TRegisteredObj<_Ty>>() {}
	template<typename _Ty>
	TRegisteredPointer<_Ty>::TRegisteredPointer(TRegisteredObj<_Ty>* ptr) : TSaferPtr<TRegisteredObj<_Ty>>(ptr) {
		if (nullptr != ptr) {
			(*ptr).mseRPManager().registerPointer(*this);
		}
	}
	template<typename _Ty>
	TRegisteredPointer<_Ty>::TRegisteredPointer(const TRegisteredPointer& src_cref) : TSaferPtr<TRegisteredObj<_Ty>>(src_cref.m_ptr) {
		if (nullptr != src_cref.m_ptr) {
			(*(src_cref.m_ptr)).mseRPManager().registerPointer(*this);
		}
	}
	template<typename _Ty>
	TRegisteredPointer<_Ty>::~TRegisteredPointer() {
		if (nullptr != (*this).m_ptr) {
			(*((*this).m_ptr)).mseRPManager().unregisterPointer(*this);
		}
		mseRPManager().onObjectDestruction(); /* Just in case there are pointers to this pointer out there. */
	}
	template<typename _Ty>
	TRegisteredPointer<_Ty>& TRegisteredPointer<_Ty>::operator=(TRegisteredObj<_Ty>* ptr) {
		if (nullptr != (*this).m_ptr) {
			(*((*this).m_ptr)).mseRPManager().unregisterPointer(*this);
		}
		TSaferPtr<TRegisteredObj<_Ty>>::operator=(ptr);
		if (nullptr != ptr) {
			(*ptr).mseRPManager().registerPointer(*this);
		}
		return (*this);
	}
	template<typename _Ty>
	TRegisteredPointer<_Ty>& TRegisteredPointer<_Ty>::operator=(const TRegisteredPointer<_Ty>& _Right_cref) {
		return operator=(_Right_cref.m_ptr);
	}
	/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
	template<typename _Ty>
	TRegisteredPointer<_Ty>::operator _Ty*() const {
		if (nullptr == (*this).m_ptr) {
			int q = 5; /* just a line of code for putting a debugger break point */
		}
		return (*this).m_ptr;
	}

	/* registered_new is intended to be analogous to std:make_shared */
	template <class _Ty, class... Args>
	TRegisteredPointer<_Ty> registered_new(Args&&... args) {
		return new TRegisteredObj<_Ty>(args...);
	}
	template <class _Ty>
	void registered_delete(const TRegisteredPointer<_Ty>& regPtrRef) {
		auto a = dynamic_cast<TRegisteredObj<_Ty> *>((_Ty *)regPtrRef);
		delete a;
	}


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

	/* TRegisteredPointerForLegacy is similar to TRegisteredPointer, but more readily converts to a native pointer implicitly. So
	when replacing native pointers with "registered" pointers in legacy code, fewer code changes (explicit casts) may be required
	when using this template. */
	template<typename _Ty>
	class TRegisteredPointerForLegacy : public TSaferPtrForLegacy<_Ty> {
	public:
		TRegisteredPointerForLegacy() : TSaferPtrForLegacy<_Ty>() { /*gSPManager.registerPointer((*this), null_ptr);*/ }
		TRegisteredPointerForLegacy(_Ty* ptr) : TSaferPtrForLegacy<_Ty>(ptr) { gSPManager.registerPointer((*this), ptr); }
		TRegisteredPointerForLegacy(const TRegisteredPointerForLegacy& src_cref) : TSaferPtrForLegacy<_Ty>(src_cref.m_ptr) { gSPManager.registerPointer((*this), src_cref.m_ptr); }
		virtual ~TRegisteredPointerForLegacy() {
			gSPManager.unregisterPointer((*this), (*this).m_ptr);
			gSPManager.onObjectDestruction(this); /* Just in case there are pointers to this pointer out there. */
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

	template<typename _Ty>
	class TRegisteredObjForLegacy : public _Ty {
	public:
		//using _Ty::_Ty;
		// the version of the compiler (msvc 2013) being used does not yet support inherited constructors, so we use this macro hack
		// for now
		MSE_USING(TRegisteredObjForLegacy, _Ty);
		virtual ~TRegisteredObjForLegacy() {
			gSPManager.onObjectDestruction(this);
		}
		TRegisteredPointerForLegacy<_Ty> operator&() {
			return TRegisteredPointerForLegacy<_Ty>(this);
		}
		TRegisteredPointer<const _Ty> operator&() const {
			return TRegisteredPointer<const _Ty>(this);
		}
	};

	template <class _Ty, class... Args>
	TRegisteredPointerForLegacy<_Ty> registered_new_for_legacy(Args&&... args) {
		return new TRegisteredObjForLegacy<_Ty>(args...);
	}
	template <class _Ty>
	void registered_delete_for_legacy(const TRegisteredPointerForLegacy<_Ty>& regPtrRef) {
		auto a = dynamic_cast<TRegisteredPointerForLegacy<_Ty> *>((_Ty *)regPtrRef);
		delete a;
	}
}


#endif // MSEREGISTERED_H_

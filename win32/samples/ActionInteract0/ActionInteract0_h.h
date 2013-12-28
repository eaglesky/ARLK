

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Mon May 20 15:02:53 2013
 */
/* Compiler settings for ActionInteract0.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __ActionInteract0_h_h__
#define __ActionInteract0_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IActionInteract0_FWD_DEFINED__
#define __IActionInteract0_FWD_DEFINED__
typedef interface IActionInteract0 IActionInteract0;
#endif 	/* __IActionInteract0_FWD_DEFINED__ */


#ifndef __ActionInteract0_FWD_DEFINED__
#define __ActionInteract0_FWD_DEFINED__

#ifdef __cplusplus
typedef class ActionInteract0 ActionInteract0;
#else
typedef struct ActionInteract0 ActionInteract0;
#endif /* __cplusplus */

#endif 	/* __ActionInteract0_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __ActionInteract0_LIBRARY_DEFINED__
#define __ActionInteract0_LIBRARY_DEFINED__

/* library ActionInteract0 */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_ActionInteract0;

#ifndef __IActionInteract0_DISPINTERFACE_DEFINED__
#define __IActionInteract0_DISPINTERFACE_DEFINED__

/* dispinterface IActionInteract0 */
/* [uuid] */ 


EXTERN_C const IID DIID_IActionInteract0;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("99B35469-4B32-4F5B-BE3C-7AAC96E3BA78")
    IActionInteract0 : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IActionInteract0Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IActionInteract0 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IActionInteract0 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IActionInteract0 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IActionInteract0 * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IActionInteract0 * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IActionInteract0 * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IActionInteract0 * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } IActionInteract0Vtbl;

    interface IActionInteract0
    {
        CONST_VTBL struct IActionInteract0Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActionInteract0_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IActionInteract0_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IActionInteract0_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IActionInteract0_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IActionInteract0_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IActionInteract0_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IActionInteract0_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IActionInteract0_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_ActionInteract0;

#ifdef __cplusplus

class DECLSPEC_UUID("F7A5A312-23EB-43BF-AE7B-B8858D8ED07B")
ActionInteract0;
#endif
#endif /* __ActionInteract0_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif



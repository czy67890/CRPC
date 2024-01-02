//
// Created by chenzeyu on 2024/1/1.
//

#ifndef CZYSERVER_ANY_INVOCABLE_H
#define CZYSERVER_ANY_INVOCABLE_H
#include <type_traits>
#include <cstddef>
#include <functional>
#include <memory>


template<class Sig>
        class AnyInvocable;

namespace internal_any_invocable{
    enum StorageProperty :std::size_t {
        kAlignment = alignof(std::max_align_t),
        kStorageSize = sizeof(void * ) * 2
    };

    template <class T>
    struct IsAnyInvocable :std::false_type {};

    template <class Sig>
    struct IsAnyInvocable<AnyInvocable<Sig>> :std::true_type {};

    template <class T>
    using IsStoredLocally = std::integral_constant<bool,sizeof(T) <= kStorageSize && alignof(T) <= kAlignment && kAlignment %
                                                                                                                         alignof(T) == 0 && std::is_nothrow_move_constructible_v<T>>;
    template<class T>
    using RemoveCVRef = typename std::remove_cv<typename std::remove_reference<T>::type>::type ;

    template <class ReturnType,class F,class ...P,typename = std::enable_if_t<std::is_void_v<ReturnType>>>
    void InvokeR(F && f,P&& ... args){
        std::invoke(std::forward<F>(f),std::forward<P>(args)...);
    }

    template <class ReturnType,class F,class ...P,std::enable_if_t<!std::is_void_v<ReturnType>,int> = 0>
    ReturnType InvokeR(F && f,P && ... args){
        return std::invoke(std::forward<F>(f),std::forward<P>(args)...);
    }

    template<typename T>
    T ForwardImpl(std::true_type);

    template<typename T>
    T && ForwardImpl(std::false_type);

    /// when the T is num or ptr or enum
    /// we copy it
    /// other_wise
    /// we forward it
    template<typename T>
    struct ForwardedParameter{
        using type = decltype((ForwardImpl<T>(std::integral_constant<bool,std::is_scalar_v<T>>())));
    };

    template<typename T>
    using ForwardedParameterType = ForwardedParameter<T>::type;

     enum class FunctionToCall:bool{
            relocate_from_to,
            dispose
     };


     /// a common optimize for small obj
     union TypeEraseState{
         struct {
             void * target;
             size_t size;
         } remote;
         alignas(kAlignment) char storage[kStorageSize];
     };

     template <class T>
    T & ObjectInLocalStorage(TypeEraseState * const state){
       return *std::launder(reinterpret_cast<T*>(state->storage));
    }

    using ManagerType = void(FunctionToCall ,TypeEraseState *from ,TypeEraseState * to) noexcept;

     template<bool SigNoexcept,class ReturnType,class ... P>
     using InvokerType = ReturnType(TypeEraseState *,ForwardedParameterType<P>...);

     inline void EmptyManager(FunctionToCall,TypeEraseState* ,TypeEraseState *) noexcept{}

     inline void LocalManagerTrivial(FunctionToCall,TypeEraseState *from,TypeEraseState *to)noexcept{
         *to = *from;
     }

     template<class T>
     void LocalManagerNontrivial(FunctionToCall operation,TypeEraseState * const from ,TypeEraseState * const to) noexcept{
         static_assert(IsStoredLocally<T>::value,"Local storage must only be used for supported types.");
         static_assert(!std::is_trivially_copyable_v<T>,"Locally stored types must be trivially copyable");

         T& from_object = (ObjectInLocalStorage<T>(from));
         switch(operation){
             case FunctionToCall::relocate_from_to:
                 ::new (static_cast<void*> (&to->storage)) T(std::move(from_object));
                 [[fallthrough]];
             case FunctionToCall::dispose:
                from_object.~T();
                return;
         }
     }

     template <bool SigIsNoexcept,class ReturnType,class QualTRef,class ... P>
     ReturnType LocalInvoker(TypeEraseState * const state,ForwardedParameterType<P>... args) noexcept(SigIsNoexcept){
         using RawT = RemoveCVRef<QualTRef>;
         static_assert(IsStoredLocally<RawT>::value,"target callable object must can be stored in local state");
         auto & f = ObjectInLocalStorage<RawT>(state);
         return (InvokeR<ReturnType>(static_cast<QualTRef>(f),static_cast<ForwardedParameterType<P>>(args)...));
     }

     inline void RemoteManagerTrivial(FunctionToCall operation,TypeEraseState * const from,TypeEraseState * const to) noexcept{
         switch (operation) {
             case FunctionToCall::relocate_from_to:
                 to->remote = from->remote;
                 return;
             case FunctionToCall::dispose:
                ::operator delete(from->remote.target,from->remote.size);
                return;
         }
     }

     template <typename T>
     void RemoteManagerNontrivial(FunctionToCall operation,TypeEraseState *const from,TypeEraseState * const to ) noexcept{
         switch(operation){
             case FunctionToCall::relocate_from_to:
                 to->remote = from->remote;
                 return;
             case FunctionToCall::dispose:
                 ::delete static_cast<T*>(to->remote.target);
                 return;
         }
     }

     template <bool SigIsNoexcept,class ReturnType ,class QualType,class ...P>
     ReturnType RemoteInvoker(TypeEraseState *const state,ForwardedParameterType<P>... args) noexcept{
         using RawT = RemoveCVRef<QualType>;
         static_assert(!IsStoredLocally<RawT>::value,"this must be called by the not local stored class");
         auto &f = *static_cast<RawT*>(state->remote.target);
         return (InvokeR<ReturnType>(static_cast<QualType>(f),static_cast<ForwardedParameterType<P>>(args)...));
     }

     template<class T>
     struct IsInPlaceType :std::false_type {};

     template<class T>
     struct IsInPlaceType<std::in_place_type_t<T>> :std::true_type {};

     template <class QualDecayedTRef>
     struct TypedConversionConstruct{};

     template<class Sig>
     class Impl{};
#if defined(__cpp_sized_deallocation)

     class TrivialDeleter{
     public:
         explicit TrivialDeleter(std::size_t size)
            :size_(size)
         {
         }

         void operator()(void *target) const {
             ::operator delete(target,size_);
         }
     private:
         size_t size_;
     };
#else
    class TrivialDeleter{
     public:
         explicit TrivialDeleter(std::size_t size)

         {
         }

         void operator()(void *target) const {
             ::operator delete(target);
         }
     private:
     };
#endif



    template<bool SigIsNoexpcet,class ReturnType,class ... P>
    class CoreImpl;

    constexpr bool IsCompatibleConVersion(void *,void *) {
        return false;
    }





    template<bool NoExceptSrc,bool NoExceptDest,class ... T>
    constexpr bool IsCompatibleConVersion(CoreImpl<NoExceptSrc,T...>*,
                                          CoreImpl<NoExceptDest,T...>*){
        return !NoExceptDest || NoExceptSrc;
    }




    template<typename T>
    using HasTrivialRemoteStroage = std::integral_constant<bool,std::is_trivially_destructible<T>::value && alignof(T) <=
                                                                                                            alignof(std::max_align_t)>;
    template<bool SigIsNoexcept,class ReturnType,class ... P>
    class CoreImpl{
    public:
        template <typename Other>
        struct IsCompatibleAnyInvocable{
            static constexpr bool value{false};
        };


    using result_type = ReturnType;

        enum class TargetType{
            kPointer,
            kCompatibleAnyInvocable,
            kIncompatibleAnyInvocable,
            kOther
        };

        template<class QualDecayedTRef,class F>
        explicit CoreImpl(TypedConversionConstruct<QualDecayedTRef>,F && f){
            using DecayedT = RemoveCVRef<QualDecayedTRef>;

            constexpr TargetType kTargetType = (std::is_pointer_v<DecayedT>||
                                                std::is_member_pointer_v<DecayedT>
                                                ) ? TargetType::kPointer : ((IsCompatibleAnyInvocable<DecayedT>::value) ?
                                                TargetType::kCompatibleAnyInvocable : IsAnyInvocable<DecayedT>::value ? TargetType::kIncompatibleAnyInvocable : TargetType::kOther);


            Initializer<kTargetType,QualDecayedTRef>(std::forward<F>(f));
        }


        template<class QualTRef,class ... Args>
        explicit CoreImpl(std::in_place_type_t<QualTRef>,Args&& ... args){
            InitializeStorage<QualTRef>(std::forward<Args>(args)...);
        }

        CoreImpl(CoreImpl && rhs) noexcept{
            rhs.manager_(FunctionToCall::relocate_from_to,&rhs.state_,&state_);
            manager_ = rhs.manager_;
            invoker_ = rhs.invoker_;
            rhs.manager_ = EmptyManager;
            rhs.invoker_ = nullptr;
        }

        void Clear(){
            manager_(FunctionToCall::dispose,&state_,&state_);
            manager_  = EmptyManager;
            invoker_ = nullptr;
        }


        CoreImpl& operator=(CoreImpl &&rhs) noexcept{
            Clear();
            rhs.manager_(FunctionToCall::relocate_from_to,&rhs.state_,&state_);
            manager_ = rhs.manager_;
            invoker_ = rhs.invoker_;
            rhs.manager_ = EmptyManager;
        }

        template <class T ,typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
        void InitializeLocalManager(){
            manager_ = LocalManagerTrivial;
        }

        template<class T ,std::enable_if_t<!std::is_trivial_v<T>,int> = 0>
        void InitializeLocalManager(){
            manager_ = LocalManagerNontrivial<T>;
        }

        ~CoreImpl() noexcept{
            manager_(FunctionToCall::dispose,&state_,&state_);
        }


        bool HasValue() const{
            return invoker_ != nullptr;
        }



        template <class T,class ...Args,typename = std::enable_if_t<HasTrivialRemoteStroage<T>::value>>
        void InitializeRemoteManager(Args&&... args){
            /// use unique_ptr only for the except safety
            std::unique_ptr<void,TrivialDeleter> uninitialized_target(::operator new (sizeof(T)) , TrivialDeleter(sizeof(T)));
            :: new (uninitialized_target.get())T(std::forward<Args>(args)...);
            state_.remote.target =  uninitialized_target.release();
            state_.remote.size = sizeof(T);
            manager_ = RemoteManagerTrivial;
        }

        template <class T,class ...Args,std::enable_if_t<!HasTrivialRemoteStroage<T>::value,int> = 0>
        void InitializeRemoteManager(Args&&... args){
            state_.remote.target = ::new T(std::forward<Args>(args)...);
            manager_ = RemoteManagerNontrivial<T>;
        }

        template <class QualTRef,class ... Args,std::enable_if_t<!IsStoredLocally<RemoveCVRef<QualTRef>>::value,int> = 0>
        void InitializeStorage(Args&&...  args){
            InitializeRemoteManager<RemoveCVRef<QualTRef>>(std::forward<Args>(args)...);
            invoker_ = RemoteInvoker<SigIsNoexcept,ReturnType,QualTRef,P...>;
        }

        template <class QualTRef,class ... Args,typename = std::enable_if_t<IsStoredLocally<QualTRef>::value>>
        void InitializeStorage(Args&&...  args){
            using RawT = RemoveCVRef<QualTRef>;
            ::new (static_cast<void *> (&state_.storage)) RawT (std::forward<Args>(args)...);
            invoker_ = LocalInvoker<SigIsNoexcept,ReturnType,QualTRef,P...>;
            InitializeLocalManager<RawT>();
        }

        template <TargetType target_type,class QualDecayedTRef,class F,std::enable_if_t<target_type == TargetType::kPointer,int> = 0>
        void Initializer(F && f){
            if(static_cast<RemoveCVRef<QualDecayedTRef>> (f) == nullptr){
                manager_ = EmptyManager;
                invoker_ = nullptr;
                return;
            }
            InitializeStorage<QualDecayedTRef>(std::forward<F>(f));
        }

        template <typename Sig>
        struct IsCompatibleAnyInvocable<AnyInvocable<Sig>>{
            static constexpr bool value = (IsCompatibleConVersion)(static_cast<typename AnyInvocable<Sig>::CoreImpl*>(nullptr),
                                                               static_cast<CoreImpl*>(nullptr));
        };



        TypeEraseState state_;
        ManagerType* manager_;
        InvokerType<SigIsNoexcept,ReturnType,P...>* invoker_;
    };

    struct ConversionConstruct{

    };

    template <class T>
    struct UnwrapStdReferenceWrapperImpl{
        using type = T;
    };

    template <class T>
    struct UnwrapStdReferenceWrapperImpl<std::reference_wrapper<T>>{
        using type = T&;
    };

    template<class T >
    using UnWrapStdReferenceWrapper = typename UnwrapStdReferenceWrapperImpl<T>::type;

    template<class... T>
    using TrueAlias = std::integral_constant<bool,sizeof(std::void_t<T...>*) != 0>;

    template<class Sig,class F,typename = std::enable_if_t<!std::is_same_v<RemoveCVRef<F>,AnyInvocable<Sig>>>>
    using CanConvert = TrueAlias<std::enable_if_t<!IsInPlaceType<RemoveCVRef<F>>::value>
                                                    std::enable_if_t<Impl<Sig>>::template CallIsValid<F>::value,>                                                   ;
}





template <class T>
class AnyInvocable {
private:
    static_assert(std::is_function_v<T>,"this template argument must be a function type.");

    using Impl = T;
public:



};




#endif //CZYSERVER_ANY_INVOCABLE_H

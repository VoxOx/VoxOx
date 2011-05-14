//Window>-specific>
#ifdef _WINDOWS
// Add> C> includes> here>

// Add> C>++ includes> here>
#if defined __cplusplus

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#include <boost/any.hpp>
#include <boost/assert.hpp>
#include <boost/checked_delete.hpp>
#include <boost/config.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/iterator.hpp>
#include <boost/next_prior.hpp>
#include <boost/noncopyable.hpp>
#include <boost/operators.hpp>
#include <boost/ref.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signal.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/throw_exception.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility.hpp>
#include <boost/version.hpp>
#include <boost/weak_ptr.hpp>

//These two files must be included BEFORE any boost/serialization files
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/config/abi/msvc_prefix.hpp>
//#include <boost/config/auto_link.hpp>
#include <boost/config/compiler/visualc.hpp>
#include <boost/config/platform/win32.hpp>
#include <boost/config/requires_threads.hpp>
#include <boost/config/select_compiler_config.hpp>
#include <boost/config/select_platform_config.hpp>
#include <boost/config/select_stdlib_config.hpp>
#include <boost/config/stdlib/dinkumware.hpp>
#include <boost/config/suffix.hpp>
#include <boost/config/user.hpp>

#include <boost/detail/bad_weak_ptr.hpp>
#include <boost/detail/interlocked.hpp>
#include <boost/detail/shared_count.hpp>
#include <boost/detail/sp_counted_base.hpp>
#include <boost/detail/sp_counted_base_w32.hpp>
#include <boost/detail/sp_counted_impl.hpp>
#include <boost/detail/workaround.hpp>

#include <boost/mpl/aux_/adl_barrier.hpp>
#include <boost/mpl/aux_/config/adl.hpp>
#include <boost/mpl/aux_/config/ctps.hpp>
#include <boost/mpl/aux_/config/gcc.hpp>
#include <boost/mpl/aux_/config/intel.hpp>
#include <boost/mpl/aux_/config/lambda.hpp>
#include <boost/mpl/aux_/config/msvc.hpp>
#include <boost/mpl/aux_/config/overload_resolution.hpp>
#include <boost/mpl/aux_/config/preprocessor.hpp>
#include <boost/mpl/aux_/config/static_constant.hpp>
#include <boost/mpl/aux_/config/ttp.hpp>
#include <boost/mpl/aux_/config/workaround.hpp>
//#include <boost/mpl/aux_/integral_wrapper.hpp>
#include <boost/mpl/aux_/lambda_support.hpp>
#include <boost/mpl/aux_/nttp_decl.hpp>
#include <boost/mpl/aux_/preprocessor/params.hpp>
#include <boost/mpl/aux_/static_cast.hpp>
#include <boost/mpl/aux_/template_arity_fwd.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/bool_fwd.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/int_fwd.hpp>
#include <boost/mpl/integral_c.hpp>
#include <boost/mpl/integral_c_fwd.hpp>
#include <boost/mpl/integral_c_tag.hpp>
#include <boost/mpl/size_t.hpp>
#include <boost/mpl/size_t_fwd.hpp>

#include <boost/preprocessor/arithmetic/add.hpp>
#include <boost/preprocessor/arithmetic/dec.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
//#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/comma_if.hpp>
#include <boost/preprocessor/config/config.hpp>
#include <boost/preprocessor/control/detail/msvc/while.hpp>
#include <boost/preprocessor/control/expr_iif.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/control/iif.hpp>
#include <boost/preprocessor/control/while.hpp>
#include <boost/preprocessor/debug/error.hpp>
#include <boost/preprocessor/detail/auto_rec.hpp>
#include <boost/preprocessor/detail/check.hpp>
#include <boost/preprocessor/detail/is_binary.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/inc.hpp>
#include <boost/preprocessor/list/adt.hpp>
#include <boost/preprocessor/list/detail/fold_left.hpp>
#include <boost/preprocessor/list/detail/fold_right.hpp>
#include <boost/preprocessor/list/fold_left.hpp>
#include <boost/preprocessor/list/fold_right.hpp>
#include <boost/preprocessor/list/reverse.hpp>
#include <boost/preprocessor/logical/bitand.hpp>
#include <boost/preprocessor/logical/bool.hpp>
#include <boost/preprocessor/logical/compl.hpp>
#include <boost/preprocessor/punctuation/comma.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repeat.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/tuple/eat.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/rem.hpp>

//JRT - 2009.04.01 - MUST have the boost/archive files included BEFORE any serialization files.
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/collections_load_imp.hpp>
#include <boost/serialization/collections_save_imp.hpp>
#include <boost/serialization/collection_traits.hpp>
#include <boost/serialization/config.hpp>
#include <boost/serialization/deque.hpp>

#include <boost/serialization/export.hpp>
#include <boost/serialization/extended_type_info.hpp>
#include <boost/serialization/extended_type_info_no_rtti.hpp>
#include <boost/serialization/extended_type_info_typeid.hpp>
#include <boost/serialization/force_include.hpp>
#include <boost/serialization/hash_map.hpp>
#include <boost/serialization/hash_set.hpp>
#include <boost/serialization/is_abstract.hpp>
#include <boost/serialization/level.hpp>
#include <boost/serialization/level_enum.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/shared_ptr.hpp>

//#include <boost/serialization/shared_ptr_132.hpp>		//Conflict with QT::uint8_t

#include <boost/serialization/slist.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/tracking_enum.hpp>

#include <boost/serialization/traits.hpp>
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/void_cast.hpp>
#include <boost/serialization/void_cast_fwd.hpp>
#include <boost/serialization/weak_ptr.hpp>

//#include <boost/serialization/detail/shared_count_132.hpp>
//#include <boost/serialization/detail/shared_ptr_132.hpp>
//#include <boost/serialization/detail/shared_ptr_nmt_132.hpp>
#include <boost/serialization/detail/stack_constructor.hpp>


#include <boost/signals/connection.hpp>
//#include <boost/signals/signal_template.hpp>
#include <boost/signals/signal0.hpp>
#include <boost/signals/signal1.hpp>
#include <boost/signals/signal2.hpp>
#include <boost/signals/signal3.hpp>
#include <boost/signals/signal4.hpp>
#include <boost/signals/signal5.hpp>
#include <boost/signals/signal6.hpp>
#include <boost/signals/signal7.hpp>
#include <boost/signals/signal8.hpp>
#include <boost/signals/signal9.hpp>
#include <boost/signals/signal10.hpp>
#include <boost/signals/slot.hpp>
#include <boost/signals/trackable.hpp>
#include <boost/signals/detail/config.hpp>
#include <boost/signals/detail/signals_common.hpp>

#include <boost/thread/detail/config.hpp>
#include <boost/thread/detail/lock.hpp>
#include <boost/thread/exceptions.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/add_cv.hpp>
#include <boost/type_traits/add_pointer.hpp>
#include <boost/type_traits/add_reference.hpp>
#include <boost/type_traits/add_volatile.hpp>
#include <boost/type_traits/alignment_of.hpp>
#include <boost/type_traits/broken_compiler_spec.hpp>
#include <boost/type_traits/config.hpp>
#include <boost/type_traits/conversion_traits.hpp>
#include <boost/type_traits/detail/bool_trait_def.hpp>
#include <boost/type_traits/detail/bool_trait_undef.hpp>
#include <boost/type_traits/detail/cv_traits_impl.hpp>
#include <boost/type_traits/detail/false_result.hpp>
#include <boost/type_traits/detail/ice_and.hpp>
#include <boost/type_traits/detail/ice_eq.hpp>
#include <boost/type_traits/detail/ice_not.hpp>
#include <boost/type_traits/detail/ice_or.hpp>
#include <boost/type_traits/detail/is_function_ptr_tester.hpp>
#include <boost/type_traits/detail/is_mem_fun_pointer_tester.hpp>
#include <boost/type_traits/detail/size_t_trait_def.hpp>
#include <boost/type_traits/detail/size_t_trait_undef.hpp>
#include <boost/type_traits/detail/template_arity_spec.hpp>
#include <boost/type_traits/detail/type_trait_def.hpp>
#include <boost/type_traits/detail/type_trait_undef.hpp>
#include <boost/type_traits/detail/yes_no_type.hpp>
#include <boost/type_traits/function_traits.hpp>
#include <boost/type_traits/has_nothrow_assign.hpp>
#include <boost/type_traits/has_nothrow_constructor.hpp>
#include <boost/type_traits/has_nothrow_copy.hpp>
#include <boost/type_traits/has_nothrow_destructor.hpp>
#include <boost/type_traits/has_trivial_assign.hpp>
#include <boost/type_traits/has_trivial_constructor.hpp>
#include <boost/type_traits/has_trivial_copy.hpp>
#include <boost/type_traits/has_trivial_destructor.hpp>
#include <boost/type_traits/has_virtual_destructor.hpp>
#include <boost/type_traits/ice.hpp>
#include <boost/type_traits/integral_constant.hpp>
#include <boost/type_traits/intrinsics.hpp>
#include <boost/type_traits/is_abstract.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/type_traits/is_array.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/is_class.hpp>
#include <boost/type_traits/is_compound.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/is_empty.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/type_traits/is_float.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <boost/type_traits/is_function.hpp>
#include <boost/type_traits/is_fundamental.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_member_function_pointer.hpp>
#include <boost/type_traits/is_member_pointer.hpp>
#include <boost/type_traits/is_pod.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_reference.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_scalar.hpp>
#include <boost/type_traits/is_signed.hpp>
#include <boost/type_traits/is_union.hpp>
#include <boost/type_traits/is_unsigned.hpp>
#include <boost/type_traits/is_void.hpp>
#include <boost/type_traits/is_volatile.hpp>
#include <boost/type_traits/remove_cv.hpp>
#include <boost/type_traits/remove_reference.hpp>

#include <boost/utility/addressof.hpp>
#include <boost/utility/base_from_member.hpp>
#include <boost/utility/enable_if.hpp>

#endif	//__cplusplus

#endif	//_WINDOWS
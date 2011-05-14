ow_create_static_library(srtp)

include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

ow_add_public_include_dirs(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/crypto/include
)

ow_add_sources(
	crypto/cipher/aes.c
	crypto/cipher/aes_cbc.c
	crypto/cipher/aes_icm.c
	crypto/cipher/cipher.c
	crypto/cipher/null_cipher.c
	crypto/hash/auth.c
	crypto/hash/hmac.c
	crypto/hash/null_auth.c
	crypto/hash/sha1.c
	crypto/replay/rdb.c
	crypto/replay/rdbx.c
	crypto/replay/ut_sim.c
	crypto/math/datatypes.c
	crypto/math/stat.c
	crypto/kernel/alloc.c
	crypto/kernel/crypto_kernel.c
	crypto/kernel/err.c
	crypto/kernel/key.c
	crypto/rng/ctr_prng.c
	crypto/rng/prng.c
	crypto/rng/rand_source.c
	srtp/srtp.c
)

if (WIN32)
	ow_add_private_definitions(
		-DWIN32
	)
endif (WIN32)

ow_create_project_binary()

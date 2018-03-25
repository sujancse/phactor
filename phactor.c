/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-present The PHP Group                             |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Thomas Punt <tpunt@php.net>                                  |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <main/php.h>
#include <main/SAPI.h>
#include <ext/standard/info.h>

#include "php_phactor.h"
#include "src/classes/actor_system.h"
#include "src/classes/actor.h"
#include "src/classes/actor_ref.h"
#include "src/classes/supervisor.h"

#ifndef ZTS
# error "Zend Thread Safety (ZTS) mode is required"
#endif

#if !defined(ZEND_ENABLE_STATIC_TSRMLS_CACHE) || !ZEND_ENABLE_STATIC_TSRMLS_CACHE
# error "TSRMLS static cache is required"
#endif

#if defined(PH_FIXED_STACK_SIZE) && PH_FIXED_STACK_SIZE < 8192
# error "The fixed stack size must be at least 8192 bytes"
#endif

extern pthread_mutex_t global_actor_id_lock;
extern pthread_mutex_t global_tree_number_lock;

common_strings_t common_strings;

ZEND_DECLARE_MODULE_GLOBALS(phactor)

PHP_MINIT_FUNCTION(phactor)
{
    ph_actor_system_ce_init();
    ph_actor_ce_init();
    ph_actor_ref_ce_init();
    ph_supervisor_ce_init();

    pthread_mutex_init(&global_actor_id_lock, NULL);
    pthread_mutex_init(&global_tree_number_lock, NULL);

    common_strings.receive = zend_string_init(ZEND_STRL("receive"), 1);
    GC_FLAGS(common_strings.receive) |= IS_STR_INTERNED;
    common_strings.__construct = zend_string_init(ZEND_STRL("__construct"), 1);
    GC_FLAGS(common_strings.__construct) |= IS_STR_INTERNED;
    common_strings.ref = zend_string_init(ZEND_STRL("ref"), 1);
    GC_FLAGS(common_strings.ref) |= IS_STR_INTERNED;
    common_strings.name = zend_string_init(ZEND_STRL("name"), 1);
    GC_FLAGS(common_strings.name) |= IS_STR_INTERNED;

    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(phactor)
{
    pthread_mutex_destroy(&global_actor_id_lock);
    pthread_mutex_destroy(&global_tree_number_lock);

    GC_FLAGS(common_strings.receive) &= ~IS_STR_INTERNED;
    zend_string_free(common_strings.receive);
    GC_FLAGS(common_strings.__construct) &= ~IS_STR_INTERNED;
    zend_string_free(common_strings.__construct);
    GC_FLAGS(common_strings.ref) &= ~IS_STR_INTERNED;
    zend_string_free(common_strings.ref);
    GC_FLAGS(common_strings.name) &= ~IS_STR_INTERNED;
    zend_string_free(common_strings.name);

    return SUCCESS;
}

PHP_RINIT_FUNCTION(phactor)
{
    TSRMLS_CACHE_UPDATE();

    zend_hash_init(&PHACTOR_ZG(op_array_file_names), 8, NULL, ZVAL_PTR_DTOR, 0);
    PHACTOR_ZG(allowed_to_construct_object) = 0;

    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(phactor)
{
    zend_hash_destroy(&PHACTOR_ZG(op_array_file_names));

    return SUCCESS;
}

PHP_MINFO_FUNCTION(phactor)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "phactor support", "enabled");
    php_info_print_table_end();
}

zend_module_entry phactor_module_entry = {
    STANDARD_MODULE_HEADER,
    "phactor",
    NULL,
    PHP_MINIT(phactor),
    PHP_MSHUTDOWN(phactor),
    PHP_RINIT(phactor),
    PHP_RSHUTDOWN(phactor),
    PHP_MINFO(phactor),
    PHP_PHACTOR_VERSION,
    PHP_MODULE_GLOBALS(phactor),
    NULL,
    NULL,
    NULL,
    STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_PHACTOR
TSRMLS_CACHE_DEFINE()
ZEND_GET_MODULE(phactor)
#endif

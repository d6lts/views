<?php
// $Id$
/**
 * @file
 * This file contains no working PHP code; it exists to provide additional documentation
 * for doxygen as well as to document hooks in the standard Drupal manner.
 */

/**
 * @mainpage Views 2 API Manual
 *
 * This is a stub for the main page.
 *
 * Topics:
 * - @ref view_lifetime
 * - @ref views_hooks
 * - @ref views_handlers
 * - @ref views_plugins
 * - @ref views_templates
 */

/**
 * @page view_lifetime The life of a view
 *
 * This page explains the basic cycle of a view and what processes happen.
 */

/**
 * @page views_handlers About Views' handlers
 *
 * This page explains what views handlers are, how they're written, and what
 * the basic conventions are.
 *
 * - @ref views_field_handlers
 * - @ref views_sort_handlers
 * - @ref views_filter_handlers
 * - @ref views_argument_handlers
 * - @ref views_relationship_handlers
 */

/**
 * @page views_plugins About Views' plugins
 *
 * This page explains what views plugins are, how they're written, and what
 * the basic conventions are.
 *
 * - @ref views_display_plugins
 * - @ref views_style_plugins
 * - @ref views_row_plugins
 */

/**
 * @defgroup views_hooks Views' hooks
 * @{
 * Hooks that can be implemented by other modules in order to implement the
 * Views API.
 */

/**
 * Stub hook documentation
 *
 * This hook should be placed in MODULENAME.views.inc and it will be auto-loaded.
 * This must either be in the same directory as the .module file or in a subdirectory
 * named 'includes'.
 */
function hook_views_data() {
  // example code here
}

/**
 * Stub hook documentation
 *
 * This hook should be placed in MODULENAME.views.inc and it will be auto-loaded.
 * This must either be in the same directory as the .module file or in a subdirectory
 * named 'includes'.
 */
function hook_views_plugins() {
  // example code here
}

/**
 * Stub hook documentation
 *
 * This hook should be placed in MODULENAME.views_default.inc and it will be auto-loaded.
 * This must either be in the same directory as the .module file or in a subdirectory
 * named 'includes'.
 */
function hook_views_default_views() {
  // example code here
}

/**
 * Stub hook documentation
 *
 * This hook should be placed in MODULENAME.views_convert.inc and it will be auto-loaded.
 * This must either be in the same directory as the .module file or in a subdirectory
 * named 'includes'.
 */
function hook_views_convert() {
  // example code here
}

/**
 * Stub hook documentation
 */
function hook_views_query_substitutions() {
  // example code here
}

/**
 * Stub hook documentation
 *
 * This hook should be placed in MODULENAME.views.inc and it will be auto-loaded.
 * This must either be in the same directory as the .module file or in a subdirectory
 * named 'includes'.
 *
 * Adding output to the view cam be accomplished by placing text on
 * $view->attachment_before and $view->attachment_after
 */
function hook_views_pre_view(&$view, &$display_id, &$args) {
  // example code here
}

/**
 * Stub hook documentation
 *
 * This hook should be placed in MODULENAME.views.inc and it will be auto-loaded.
 * This must either be in the same directory as the .module file or in a subdirectory
 * named 'includes'.
 *
 */
function hook_views_query_alter(&$view, &$query) {
  // example code here
}

/**
 * @}
 */

<?php
// $Id$
/**
 * @file cache.inc
 *
 * Functions to load Views' data so that it knows what is available to
 * build queries from.
 */

/**
 * Load views files on behalf of modules.
 */
function _views_include_handlers() {
  $views_path = drupal_get_path('module', 'views') . '/modules';
  foreach (module_list() as $module) {
    $module_path = drupal_get_path('module', $module);
    if (file_exists("$module_path/$module.views.inc")) {
      include_once "$module_path/$module.views.inc";
    }
    else if (file_exists("$views_path/$module.views.inc")) {
      include_once "$views_path/$module.views.inc";
    }
  }
}

/**
 * Load default views files on behalf of modules.
 */
function _views_include_default_views() {
  // Now include the views_default files specifically.
  $views_path = drupal_get_path('module', 'views') . '/modules';
  foreach (module_list() as $module) {
    $module_path = drupal_get_path('module', $module);
    if (file_exists("$module_path/$module.views_default.inc")) {
      include_once "$module_path/$module.views_default.inc";
    }
    else if (file_exists("$views_path/$module.views_default.inc")) {
      include_once "$views_path/$module.views_default.inc";
    }
  }
}

/**
 * Fetch Views' data from the cache
 */
function _views_fetch_data($table = NULL) {
  static $cache = NULL;
  if (!isset($cache)) {
    $start = microtime();
    // NOTE: This happens whether we retrieve them from cache or otherwise.
    views_include_handlers();

    // @todo: We can probably cache the views_data longer than just the current page run.
    $cache = module_invoke_all('views_data');
    if (module_exists('devel')) {
      dsm('Views data build time: ' . (microtime() - $start) * 1000 . ' ms');
    }
  }

  if (!$table) {
    return $cache;
  }
  if (isset($cache[$table])) {
    return $cache[$table];
  }
  // Return an empty array if there is no match.
  return array();
}

/**
 * Fetch the plugin data from cache.
 */
function _views_fetch_plugin_data($type = NULL, $plugin = NULL) {
  static $cache = NULL;
  if (!isset($cache)) {
    $start = microtime();
    views_include('plugins');
    views_include_handlers();

    $cache = views_discover_plugins();

    if (module_exists('devel')) {
      dsm('Views plugins build time: ' . (microtime() - $start) * 1000 . ' ms');
    }
  }

  if (!$type && !$plugin) {
    return $cache;
  }
  else if (!$plugin) {
    // Not in the if above so the else below won't run
    if (isset($cache[$type])) {
      return $cache[$type];
    }
  }
  else if (isset($cache[$type][$plugin])) {
    return $cache[$type][$plugin];
  }

  // Return an empty array if there is no match.
  return array();
}

/**
 * Scan all modules for default views and rebuild the default views cache.
 *
 * @return An associative array of all known default views.
 */
function _views_discover_default_views() {
  global $language;
  views_include_default_views();
  $defaults = module_invoke_all('views_default_views');

  foreach ($defaults as $name => $view) {
    if ($should_cache) {
      views_cache_set('views_default_views:'. $name, $view, TRUE);
    }
    $cache[$name] = $view;
  }

  return $cache;
}

/**
 * Set a cached item in the views cache.
 *
 * This is just a convenience wrapper around cache_set().
 *
 * @param $cid
 *   The cache ID of the data to store.
 * @param $data
 *   The data to store in the cache. Complex data types will be automatically serialized before insertion.
 *   Strings will be stored as plain text and not serialized.
 * @param $use_language
 *   If TRUE, the data will be cached specific to the currently active language.
 */
function views_cache_set($cid, $data, $use_language = FALSE) {
  global $language;

  if (variable_get('views_skip_cache', FALSE)) {
    return;
  }
  if ($use_language) {
    $cid .= ':'. $language->language;
  }
  cache_set($cid, $data, 'cache_views');
}

/**
 * Return data from the persistent views cache.
 *
 * This is just a convenience wrapper around cache_get().
 *
 * @param $cid
 *   The cache ID of the data to retrieve.
 * @param $use_language
 *   If TRUE, the data will be requested specific to the currently active language.
 */
function views_cache_get($cid, $use_language = FALSE) {
  global $language;

  if (variable_get('views_skip_cache', FALSE)) {
    return 0;
  }
  if ($use_language) {
    $cid .= ':'. $language->language;
  }

  return cache_get($cid, 'cache_views');
}

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
  static $cache = NULL;

  if (!isset($cache)) {
    global $language;
    $data = cache_get('views_default_views:' . $language, 'cache_views');

    if (isset($data->data)) {
      $cache = $data->data;
    }
    else {
      views_include_default_views();
      $defaults = module_invoke_all('views_default_views');
      $cache = array();

      foreach ($defaults as $name => $view) {
        $cache[$name] = $view;
      }
      cache_set('views_default_views:' . $language, $cache, 'cache_views');
    }
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

/**
 * @defgroup views_object_cache Non-volatile cache storage
 * @{
 * The non-volatile object cache is used to store an object while it is
 * being edited, so that we don't have to save until we're completely
 * done. The cache should be 'cleaned' on a regular basis, meaning to
 * remove old objects from the cache, but otherwise the data in this
 * cache must remain stable, as it includes unsaved changes.
 */

/**
 * Get an object from the non-volatile Views cache.
 *
 * This function caches in memory as well, so that multiple calls to this
 * will not result in multiple database reads.
 *
 * @param $obj
 *   A 32 character or less string to define what kind of object is being
 *   stored; primarily this is used to prevent collisions.
 * @param $name
 *   The name of the view (or other object) being stored.
 * @param $skip_cache
 *   Skip the memory cache, meaning this must be read from the db again.
 *
 * @return
 *   The data that was cached.
 */
function views_object_cache_get($obj, $name, $skip_cache = FALSE) {
  static $cache = array();
  $key = "$obj:$name";
  if ($skip_cache) {
    unset($cache[$key]);
  }

  if (!array_key_exists($key, $cache)) {
    $data = db_fetch_object(db_query("SELECT * FROM {views_object_cache} WHERE sid = '%s' AND obj = '%s' AND name = '%s'", session_id(), $obj, $name));
    if ($data) {
      $cache[$key] = unserialize($data->data);
    }
  }
  return isset($cache[$key]) ? $cache[$key] : NULL;
}

/**
 * Store an object in the non-volatile Views cache.
 *
 * @param $obj
 *   A 32 character or less string to define what kind of object is being
 *   stored; primarily this is used to prevent collisions.
 * @param $name
 *   The name of the view (or other object) being stored.
 * @param $cache
 *   The object to be cached. This will be serialized prior to writing.
 */
function views_object_cache_set($obj, $name, $cache) {
  views_object_cache_clear($obj, $name);
  db_query("INSERT INTO {views_object_cache} (sid, obj, name, data, updated) VALUES ('%s', '%s', '%s', '%s', %d)", session_id(), $obj, $name, serialize($cache), time());
}

/**
 * Remove an object from the non-volatile Views cache
 *
 * @param $obj
 *   A 32 character or less string to define what kind of object is being
 *   stored; primarily this is used to prevent collisions.
 * @param $name
 *   The name of the view (or other object) being stored.
 */
function views_object_cache_clear($obj, $name) {
  db_query("DELETE FROM {views_object_cache} WHERE sid = '%s' AND obj = '%s' AND name = '%s'", session_id(), $obj, $name);
}

/**
 * Remove all objects in the object cache that are older than the
 * specified age.
 *
 * @param $age
 *   The minimum age of objects to remove, in seconds. For example, 86400 is
 *   one day. Defaults to 7 days.
 */
function views_object_cache_clean($age = NULL) {
  if (empty($age)) {
    $age = 86400 * 7; // 7 days
  }
  db_query("DELETE FROM {views_object_cache} WHERE updated < %d", time() - $age);
}

/**
 * @}
 */

<?php
// $Id$

// Load all our module 'on behalfs'.
// Whenever we decide we need this, load our module 'on behalfs'.
$path = drupal_get_path('module', 'views') . '/modules';
$files = drupal_system_listing('views_.*\.inc$', $path, 'name', 0);

foreach($files as $file) {
  // The filename format is very specific. It must be views_MODULENAME.inc
  $module = substr_replace($file->name, '', 0, 6);
  if (module_exists($module)) {
    require_once("./$file->filename");
  }
}

// ---------------------------------------------------------------------------
// Acquire Views Data

/**
 * Return the arguments array; construct one if we haven't already. The
 * array is cached in a global, safely named variable so that arguments
 * are only constructed once per run.
 */
function _views_get_arguments($titles = false) {
  static $views_arguments;
  global $locale;

  if (!$views_arguments) {
    $data = cache_get("views_arguments:$locale", 'cache_views');
    $cache = unserialize($data->data);
    if (is_array($cache)) {
      $views_arguments = $cache;
    }
    else {
      $arguments = module_invoke_all('views_arguments');
      // allow modules to alter the definitions supplied others
      foreach (module_implements('views_arguments_alter') as $module) {
        $function = $module . '_views_arguments_alter';
        $function($arguments);
      }

      uasort($arguments, '_views_sort_arrays');
      foreach ($arguments as $name => $arg) {
        if ($arg['option'] && !is_array($arg['option'])) {
          if ($arg['option'] == 'string' || $arg['option'] == 'integer') {
            $arg['option'] = array('#type' => 'textfield', '#size' => 10, '#maxlength' => 255);
          }
          else {
            $arg['option'] = array('#type' => 'select', '#options' => $arg['option']);
          }
        }
        $views_arguments['base'][$name] = $arg['name'];
        $views_arguments['title'][$name] = $arg;
      }
      $cache = $views_arguments;
      cache_set("views_arguments:$locale", 'cache_views', serialize($cache));
    }
  }
  return ($titles ? $views_arguments['base'] : $views_arguments['title']);
}

/**
 * Constructs the full table information array. Caches it into a global array
 * so that it will only be called once per run.
 */
function _views_get_tables($full = false) {
  static $views_tables;
  global $locale;

  if (!$views_tables) {
    $data = cache_get("views_tables:$locale", 'cache_views');
    $cache = unserialize($data->data);

    if (is_array($cache)) {
      $views_tables = $cache;
    }
    else {
      $table_data = module_invoke_all('views_tables');
      $views_tables['tables'] = $table_data;
      // allow modules to alter the definitions supplied others
      foreach (module_implements('views_tables_alter') as $module) {
        $function = $module . '_views_tables_alter';
        $function($table_data);
      }

      foreach ($table_data as $name => $table) {
        if (is_array($table['filters'])) {
          foreach ($table['filters'] as $filter => $data) {
            $data['table'] = $name;
            // translate for deprecated APIs...
            if ($data['option'] && !is_array($data['option'])) {
              if ($data['option'] == 'string' || $data['option'] == 'integer') {
                $data['option'] = array('#type' => 'textfield', '#size' => 10, '#maxlength' => 255);
              }
              else {
                $data['option'] = array('#type' => 'select', '#options' => $data['option']);
              }
            }
            if ($data['list']) {
              $data['value'] = array('#type' => 'select', '#options' => $data['list'], '#validate' => array('views_filter_validate_array' => $data['name']));
              if ($data['list-type'] != 'select') {
                $data['value']['#multiple'] = TRUE;
              }
            }
            else if (!$data['value']) {
              $data['value'] = array('#type' => 'textfield', '#size' => 10, '#maxlength' => 255);
            }
            $views_tables['filters']['titles']["$name.$filter"] = $data['name'];
            $views_tables['filters']['base']["$name.$filter"] = $data;
          }
        }
        if (is_array($table['fields'])) {
          foreach ($table['fields'] as $field => $data) {
            if ($data['option'] && !is_array($data['option'])) {
              if ($data['option'] == 'string' || $data['option'] == 'integer') {
                $data['option'] = array('#type' => 'textfield', '#size' => 10, '#maxlength' => 255);
              }
              else {
                $data['option'] = array('#type' => 'select', '#options' => $data['option']);
              }
            }
            $data['table'] = $name;
            $views_tables['fields']['titles']["$name.$field"] = $data['name'];
            $views_tables['fields']['base']["$name.$field"] = $data;
          }
        }
        if (is_array($table['sorts'])) {
          foreach ($table['sorts'] as $field => $data) {
            $data['table'] = $name;
            if ($data['option'] && !is_array($data['option'])) {
              if ($data['option'] == 'string' || $data['option'] == 'integer') {
                $data['option'] = array('#type' => 'textfield', '#size' => 10, '#maxlength' => 255);
              }
              else {
                $data['option'] = array('#type' => 'select', '#options' => $data['option']);
              }
            }
            $views_tables['sorts']['titles']["$name.$field"] = $data['name'];
            $views_tables['sorts']['base']["$name.$field"] = $data;
          }
        }
      }
      cache_set("views_tables:$locale", 'cache_views', serialize($views_tables));
    }
  }
  return ($full ? $views_tables : $views_tables['tables']);
}

/**
 * Gets the filter information; if it doesn't exist, call the function
 * that constructs all that.
 */
function _views_get_filters($titles = false) {
  $table_data = _views_get_tables(true);
  return ($titles ? $table_data['filters']['titles'] : $table_data['filters']['base']);
}

/**
 * Gets the field information; if it doesn't exist, call the function
 * that constructs all that.
 */
function _views_get_fields($titles = false) {
  $table_data = _views_get_tables(true);
  return ($titles ? $table_data['fields']['titles'] : $table_data['fields']['base']);
}

/**
 * Gets the sort information; if it doesn't exist, call the function
 * that constructs all that.
 */
function _views_get_sorts($titles = false) {
  $table_data = _views_get_tables(true);
  return ($titles ? $table_data['sorts']['titles'] : $table_data['sorts']['base']);
}

/**
 * Ensures that views have legitimate information; a bit more is stored on
 * the $view object than is in the database, and this isn't necessarily
 * set when a view is constructed externally.
 */
function views_sanitize_view(&$view) {
  _views_check_arrays($view); // so reference works.
  foreach ($view->field as $i => $field) {
    if (!isset($view->field[$i]['id'])) {
      $view->field[$i]['id'] = $view->field[$i]['fullname'] = "$field[tablename].$field[field]";
      $view->field[$i]['queryname'] = "$field[tablename]_$field[field]";
    }
  }

  foreach ($view->filter as $i => $filter) {
    if (!isset($view->filter[$i]['id'])) {
      $view->filter[$i]['id'] = $view->filter[$i]['field'] = "$filter[tablename].$filter[field]";
    }
  }

  foreach ($view->exposed_filter as $i => $exposed_filter) {
    if (!isset($view->exposed_filter[$i]['id'])) {
      $view->exposed_filter[$i]['id'] = $view->exposed_filter[$i]['field'] = "$exposed_filter[tablename].$exposed_filter[field]";
    }
  }

  foreach ($view->sort as $i => $sort) {
    if (!isset($view->sort[$i]['id'])) {
      $view->sort[$i]['id'] = $view->sort[$i]['field'] = "$sort[tablename].$sort[field]";
    }
  }

  foreach ($view->argument as $i => $argument) {
    if (!isset($view->argument[$i]['id'])) {
      $view->argument[$i]['id'] = $view->argument[$i]['type'];
    }
  }
}

/**
 * Build default view information from all modules and cache it.
 */
function _views_get_default_views() {
  static $views_default_views;
  global $locale;

  if (!$views_default_views) {
    $data = cache_get("views_default_views:$locale", 'cache_views');
    $cache = unserialize($data->data);

    if (is_array($cache)) {
      $views_default_views = $cache;
    }
    else {
      // We have to make sure table data is built in order to be sure about providers.
      $tables = array_keys(_views_get_tables());

      $views = module_invoke_all('views_default_views');
      uasort($views, '_views_sort_arrays');
      $views_default_views = array();
      foreach ($views as $i => $view) {
        if (!is_array($view->requires) || !array_diff($view->requires, $tables)) {
          views_sanitize_view($view);
          $views_default_views[$i] = $view;
        }
      }
      cache_set("views_default_views:$locale", 'cache_views', serialize($views_default_views));
    }
  }
  return $views_default_views;
}

/**
 * sort callback
 */
function _views_sort_arrays($a, $b) {
  $a = (array) $a; // safety -- something send objects.
  $b = (array) $b;
  if ($a['weight'] == $b['weight']) {
    if ($a['name'] == $b['name']) {
      return 0;
    }
    return ($a['name'] < $b['name']) ? -1 : 1;
  }
  return ($a['weight'] > $b['weight']) ? -1 : 1;
}

function _views_get_query(&$view, $args) {
  if ($view->is_cacheable && ($cached = cache_get('views_query:' . $view->name, 'cache_views'))) {
    $info = unserialize($cached->data);

    $plugins = _views_get_style_plugins();
    if ($plugins[$view->type]['needs_table_header']) {
      $view->table_header = _views_construct_header($view, _views_get_fields());
    }
  }
  else {
    views_load_query();
    $info = _views_build_query($view, $args);
    if ($view->is_cacheable) {
      $data = array(
        'query' => _views_replace_args($info['query'], $info['args']),
        'countquery' => _views_replace_args($info['countquery'], $info['args']),
      );
      cache_set('views_query:' . $view->name, 'cache_views', serialize($data));
    }
  }

  // Run-time replacement so we can do cacheing
  $replacements = module_invoke_all('views_query_substitutions', $view);
  foreach ($replacements as $src => $dest) {
    $info['query'] = str_replace($src, $dest, $info['query']);
    $info['countquery'] = str_replace($src, $dest, $info['countquery']);

    if (is_array($info['args'])) {
      foreach ($info['args'] as $id => $arg) {
        $info['args'][$id] = str_replace($src, $dest, $arg);
      }
    }
  }

  return $info;
}

/**
 * Return the style plugins; construct one if we haven't already. The
 * array is cached in a static variable so that arguments
 * are only constructed once per run.
 */
function _views_get_style_plugins($titles = false) {
  static $views_style_plugins;
  global $locale;

  if (!$views_style_plugins) {
    $data = cache_get("views_style_plugins:$locale", 'cache_views');
    $cache = unserialize($data->data);
    if (is_array($cache)) {
      $views_style_plugins = $cache;
    }
    else {
      $arguments = module_invoke_all('views_style_plugins');
      uasort($arguments, '_views_sort_arrays');
      foreach ($arguments as $name => $arg) {
        if (!isset($arg['summary_theme'])) {
          $arg['summary_theme'] = 'views_summary';
        }
        $views_style_plugins['title'][$name] = $arg['name'];
        $views_style_plugins['base'][$name] = $arg;
      }
      $cache = $views_style_plugins;
      cache_set("views_style_plugins:$locale", 'cache_views', serialize($cache));
    }
  }
  return ($titles ? $views_style_plugins['title'] : $views_style_plugins['base']);
}


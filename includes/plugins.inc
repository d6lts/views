<?php
// $Id$
/**
 * @file plugins.inc
 * Built in plugins for Views output handling.
 *
 */

/**
 * Implementation of hook_views_plugins
 */
function views_views_plugins() {
  return array(
    'module' => 'views', // This just tells our themes are elsewhere.
    'display' => array(
      'default' => array(
        'title' => t('Defaults'),
        'help' => t('Default settings for this view.'),
        'handler' => 'views_display_plugin_default',
        'no ui' => TRUE,
        'no remove' => TRUE,
        'js' => array('misc/collapse.js', 'misc/textarea.js', 'misc/tabledrag.js'),
        'use pager' => TRUE,
      ),
      'page' => array(
        'title' => t('Page'),
        'help' => t('Display the view as a page, with a URL and menu links.'),
        'handler' => 'views_display_plugin_page',
        'uses hook menu' => TRUE,
        'use pager' => TRUE,
      ),
      'block' => array(
        'title' => t('Block'),
        'help' => t('Display the view as a block.'),
        'handler' => 'views_display_plugin_block',
        'uses hook block' => TRUE,
      ),
    ),
    'style' => array(
      'default' => array(
        'title' => t('Unformatted'),
        'help' => t('Displays rows one after another.'),
        'handler' => 'views_style_plugin_default',
        'theme' => 'views_view_unformatted',
        'uses row plugin' => TRUE,
      ),
      'list' => array(
        'title' => t('List'),
        'help' => t('Displays rows as an HTML list.'),
        'handler' => 'views_style_plugin_list',
        'theme' => 'views_view_list',
        'uses row plugin' => TRUE,
        'uses options' => TRUE,
      ),
/* -- not yet implemented --
      'table' => array(
        'title' => t('Table'),
        'help' => t('Displays rows in a table.'),
        'handler' => 'views_style_plugin_table',
        'theme' => 'views_view_table',
        'uses row plugin' => FALSE,
        'uses fields' => TRUE,
      ),
*/   
      'default_summary' => array(
        'title' => t('Default'),
        'help' => t('Displays the default summary view'),
        'handler' => 'views_style_plugin_summary',
        'theme' => 'views_view_summary',
        'summary' => TRUE, // only shows up as a summary style
        'uses options' => TRUE,
      ),
    ),
    'row' => array(
      'fields' => array(
        'title' => t('Fields'),
        'help' => t('Displays the fields with an optional template.'),
        'handler' => 'views_row_plugin',
        'theme' => 'views_view_fields',
        'uses fields' => TRUE,
      ),
    ),
  );

}

/**
 * Builds and return a list of all plugins available in the system.
 *
 * @return Nested array of plugins, grouped by type and
 */
function views_discover_plugins() {
  $cache = array('display' => array(), 'style' => array(), 'row' => array());
  // Get plugins from all mdoules.
  foreach (module_implements('views_plugins') as $module) {
    $function = $module . '_views_plugins';
    $result = $function();
    if (!is_array($result)) {
      continue;
    }

    $module_dir = isset($result['module']) ? $result['module'] : $module;
    // Setup automatic path/file finding for theme registration
    if ($module_dir == 'views') {
      $path = drupal_get_path('module', $module_dir) . '/theme';
      $file = 'theme.inc';
    }
    else {
      $path = drupal_get_path('module', $module_dir);
      $file = "$module.views.inc";
    }
    foreach ($result as $type => $info) {
      if ($type == 'module') {
        continue;
      }
      foreach ($info as $plugin => $def) {
        if (isset($def['theme']) && !isset($def['path'])) {
          $def['path'] = $path;
          $def['file'] = $file;
        }
        // merge the new data in
        $cache[$type][$plugin] = $def;
      }
    }
  }
  return $cache;
}

/**
 * @defgroup views_display_plugins Views' display plugins
 * @{
 * Display plugins control how Views interact with the rest of Drupal.
 *
 * They can handle creating Views from a Drupal page hook; they can
 * handle creating Views from a Drupal block hook. They can also
 * handle creating Views from an external module source, such as
 * a Panels pane, or an insert view, or a CCK field type.
 */

/**
 * The default display plugin handler. Display plugins handle options and
 * basic mechanisms for different output methods.
 */
class views_display_plugin extends views_object {
  /**
   * Fill this plugin in with the view, display, etc.
   */
  function init(&$view, &$display) {
    $this->view = $view;
    $this->display = $display;
  }

  /**
   * Determine if this display is the 'default' display which contains
   * fallback settings
   */
  function is_default_display() { return FALSE; }

  /**
   * Determine if this display uses exposed filters, so the view
   * will know whether or not to build them.
   */
  function uses_exposed() {
    if (!isset($this->has_exposed)) {
      foreach (array('field', 'filter') as $type) {
        foreach ($this->view->$type as $key => $info) {
          if ($info['handler']->is_exposed()) {
            // one is all we need; if we find it, return true.
            $this->has_exposed = TRUE;
            return TRUE;
          }
        }
      }
      $this->has_exposed = FALSE;
    }

    return $this->has_exposed;
  }

  /**
   * Does the display have a pager enabled?
   */
  function use_pager() {
    if (!empty($this->definition['use pager'])) {
      return $this->get_option('use_pager');
    }
    return FALSE;
  }

  /**
   * Static member function to list which sections are defaultable
   * and what items each section contains.
   */
  function defaultable_sections($section = NULL) {
    $sections = array(
      'access' => array('access'),
      'title' => array('title'),
      'header' => array('header', 'header_format', 'header_empty'),
      'footer' => array('footer', 'footer_format', 'footer_empty'),
      'empty' => array('empty', 'empty_format'),
      'items_per_page' => array('items_per_page', 'offset', 'use_pager', 'pager_element'),
      'use_pager' => array('items_per_page', 'offset', 'use_pager', 'pager_element'),
      'link_display' => array('link_display'),

    // @todo
      'php_arg_code' => array('php_arg_code'),
      'exposed_options' => array('exposed_options'),

      // Force these to cascade properly.
      'style_plugin' => array('style_plugin', 'style_options', 'row_plugin', 'row_options'),
      'style_options' => array('style_plugin', 'style_options', 'row_plugin', 'row_options'),
      'row_plugin' => array('style_plugin', 'style_options', 'row_plugin', 'row_options'),
      'row_options' => array('style_plugin', 'style_options', 'row_plugin', 'row_options'),

      // These guys are special
      'relationships' => array('relationships'),
      'fields' => array('fields'),
      'sorts' => array('sorts'),
      'arguments' => array('arguments'),
      'filters' => array('filters'),
    );
    if ($section) {
      if (!empty($sections[$section])) {
        return $sections[$section];
      }
    }
    else {
      return $sections;
    }
  }

  /**
   * This is called when a new display is created that has never been
   * saved to the database. It provides appropriate defaults for the
   * display, if necessary.
   */
  function options(&$display) {
    // Set the following options to use the default display, forcing
    // them to pass through unless we override.
    $display->display_options['defaults'] = array(
      'access' => TRUE,
      'title' => TRUE,
      'header' => TRUE,
      'header_format' => TRUE,
      'header_empty' => TRUE,
      'footer' => TRUE,
      'footer_format' => TRUE,
      'footer_empty' => TRUE,
      'empty' => TRUE,
      'empty_format' => TRUE,

      'items_per_page' => TRUE,
      'offset' => TRUE,
      'use_pager' => TRUE,
      'pager_element'  => TRUE,

      'link_display' => TRUE,
      'php_arg_code' => TRUE,
      'exposed_options' => TRUE,

      'style_plugin' => TRUE,
      'style_options' => TRUE,
      'row_plugin' => TRUE,
      'row_options' => TRUE,

      'relationships' => TRUE,
      'fields' => TRUE,
      'sorts' => TRUE,
      'arguments' => TRUE,
      'filters' => TRUE,
    );

    $display->display_options['relationships'] = array();
    $display->display_options['fields'] = array();
    $display->display_options['sorts'] = array();
    $display->display_options['arguments'] = array();
    $display->display_options['filters'] = array();
  }

  /**
   * Check to see if the display has a 'path' field.
   *
   * This is a pure function and not just a setting on the definition
   * because some displays (such as a panel pane) may have a path based
   * upon configuration.
   *
   * By default, displays do not have a path.
   */
  function has_path() { return FALSE; }

  /**
   * Check to see which display to use when creating links within
   * a view using this display.
   */
  function get_link_display() {
    $display_id = $this->get_option('link_display');
    // If unknown, pick the first one.
    if (empty($display_id) || empty($this->view->display[$display_id])) {
      foreach ($this->view->display as $display_id => $display) {
        if ($display->handler->has_path()) {
          return $display_id;
        }
      }
    }
    else {
      return $display_id;
    }
    // fall-through returns NULL
  }

  /**
   * Return the base path to use for this display.
   *
   * This can be overridden for displays that do strange things
   * with the path.
   */
  function get_path() {
    if ($this->has_path()) {
      return $this->get_option('path');
    }

    $display_id = $this->get_link_display();
    if ($display_id && !empty($this->view->display[$display_id])) {
      return $this->view->display[$display_id]->handler->get_path();
    }
  }

  /**
   * Check to see if the display needs a breadcrumb
   *
   * By default, displays do not need breadcrumbs
   */
  function uses_breadcrumb() { return FALSE; }

  /**
   * Intelligently get an option either from this display or from the
   * default display, if directed to do so.
   */
  function get_option($option) {
    if (!$this->is_default_display() && !empty($this->default_display) && !empty($this->display->display_options['defaults'][$option])) {
      return $this->default_display->get_option($option);
    }

    if (array_key_exists($option, $this->display->display_options)) {
      return $this->display->display_options[$option];
    }
  }

  /**
   * Intelligently set an option either from this display or from the
   * default display, if directed to do so.
   */
  function set_option($option, $value) {
    if (!$this->is_default_display() && !empty($this->default_display) && !empty($this->display->display_options['defaults'][$option])) {
      return $this->default_display->set_option($option, $value);
    }

    return $this->display->display_options[$option] = $value;
  }

  /**
   * Because forms may be split up into sections, this provides
   * an easy URL to exactly the right section. Don't override this.
   */
  function option_link($text, $section, $class = '') {
    if (!empty($class)) {
      $text = '<span>' . $text . '</span>';
    }

    return l($text, 'admin/build/views/nojs/display/' . $this->view->name . '/' . $this->display->id . '/' . $section, array('attributes' => array('class' => 'views-ajax-link ' . $class), 'html' => TRUE));
  }

  /**
   * Provide the default summary for options in the views UI.
   *
   * This output is returned as an array.
   */
  function options_summary(&$categories, &$options) {
    $categories['basic'] = array(
      'title' => t('Basic settings'),
    );

    $options['display_title'] = array(
      'category' => 'basic',
      'title' => t('Name'),
      'value' => $this->display->display_title,
    );

    $title = $this->get_option('title');
    if (!$title) {
      $title = t('None');
    }

    $options['title'] = array(
      'category' => 'basic',
      'title' => t('Title'),
      'value' => $title,
    );

    $style_plugin = views_fetch_plugin_data('style', $this->get_option('style_plugin'));
    $style_title = empty($style_plugin['title']) ? t('Missing style plugin') : $style_plugin['title'];

    $style = '';

    $options['style_plugin'] = array(
      'category' => 'basic',
      'title' => t('Style'),
      'value' => $style_title,
    );

    // This adds a 'Settings' link to the style_options setting if the style has options.
    if (!empty($style_plugin['uses options'])) {
      $options['style_plugin']['links']['style_options'] = t('Settings');
    }

    if (!empty($style_plugin['uses row plugin'])) {
      $row_plugin = views_fetch_plugin_data('row', $this->get_option('row_plugin'));
      $row_title = empty($row_plugin['title']) ? t('Missing style plugin') : $row_plugin['title'];

      $options['row_plugin'] = array(
        'category' => 'basic',
        'title' => t('Row style'),
        'value' => $row_title,
      );
      // This adds a 'Settings' link to the row_options setting if the row style has options.
      if (!empty($row_plugin['uses options'])) {
        $options['row_plugin']['links']['row_options'] = t('Settings');
      }
    }

    $access = $this->get_option('access');
    if (!is_array($access)) {
      $access = array('type' => 'none');
    }
    switch($access['type']) {
      case 'none':
      default:
        $access_str = t('Unrestricted');
        break;
      case 'perm':
        $access_str = $access['perm'];
        break;
      case 'role':
        $roles = array_keys(array_filter($access['role']));
        if (count($roles) > 1) {
          $access_str = t('Multiple roles');
        }
        else {
          $rids = views_ui_get_roles();
          $rid = array_shift($roles);
          $access_str = $rids[$rid];
        }
        break;
    }

    if (!empty($this->definition['use pager'])) {
      $options['use_pager'] = array(
        'category' => 'basic',
        'title' => t('Use pager'),
        'value' => $this->get_option('use_pager') ? t('Yes') : t('No'),
      );
    }
    $options['items_per_page'] = array(
      'category' => 'basic',
      'title' => $this->use_pager() ? t('Items per page') : t('Items to display'),
      'value' => intval($this->get_option('items_per_page')),
    );

    $options['access'] = array(
      'category' => 'basic',
      'title' => t('Access'),
      'value' => $access_str,
    );

    if (!$this->has_path()) {
      // Only show the 'link display' if there is more than one option.
      $count = 0;
      foreach ($this->view->display as $display_id => $display) {
        if ($display->handler->has_path()) {
          $count++;
        }
        if ($count > 1) {
          break;
        }
      }

      if ($count > 1) {
        $display_id = $this->get_link_display();
        $link_display = empty($this->view->display[$display_id]) ? t('None') : $this->view->display[$display_id]->display_title;
        $options['link_display'] = array(
          'category' => 'basic',
          'title' => t('Link display'),
          'value' => $link_display,
        );
      }
    }

    foreach (array('header' => t('Header'), 'footer' => t('Footer'), 'empty' => t('Empty text')) as $type => $name) {
      if (!$this->get_option($type)) {
        $field = t('None');
      }
      else {
        // A lot of code to get the name of the filter format.
        $fmt_string = $this->get_option($type . '_format');
        if (empty($fmt_string)) {
          $fmt_string = FILTER_FORMAT_DEFAULT;
        }
        $format_val = filter_resolve_format($fmt_string);
        $format = filter_formats($format_val);
        $field = $format->name;
      }

      $output[] = t('!name: !field', array('!name' => $name, '!field' => $this->option_link($field, $type)));
      $options[$type] = array(
        'category' => 'basic',
        'title' => $name,
        'value' => $field,
      );
    }
  }

  /**
   * Provide the default form for setting options.
   */
  function options_form(&$form, &$form_state) {
    if ($this->defaultable_sections($form_state['section'])) {
      $this->add_override_button($form, $form_state['section']);
    }
    $form['#title'] = check_plain($this->display->display_title) . ': ';

    // Set the 'section' to hilite on the form.
    // If it's the item we're looking at is pulling from the default display,
    // reflect that.
    if (!empty($this->display->display_options['defaults'][$form_state['section']])) {
      $form['#section'] = 'default-' . $form_state['section'];
    }
    else {
      $form['#section'] = $this->display->id . '-' . $form_state['section'];
    }

    switch ($form_state['section']) {
      case 'display_title':
        $form['#title'] .= t('The name of this display');
        $form['display_title'] = array(
          '#type' => 'textfield',
          '#description' => t('This title will appear only in the administrative interface for the View.'),
          '#default_value' => $this->display->display_title,
        );
        break;
      case 'title':
        $form['#title'] .= t('The title of this view');
        $form['title'] = array(
          '#type' => 'textfield',
          '#description' => t('This title will be displayed with the view, wherever titles are normally displayed; i.e, as the page title, block title, etc.'),
          '#default_value' => $this->get_option('title'),
        );
        break;
      case 'use_pager':
        $form['#title'] .= t('Use a pager for this view');
        $form['use_pager'] = array(
          '#type' => 'radios',
          '#options' => array(1 => t('Yes'), 0 => t('No')),
          '#default_value' => $this->get_option('use_pager'),
        );
        $form['pager_element'] = array(
          '#type' => 'textfield',
          '#title' => t('Pager element'),
          '#description' => t('Unless you\'re experiencing problems with pagers related to this view, you should leave this at 0. If using multiple pagers on one page you may need to set this number to a higher value so as not to conflict within the ?page= array.  Large values will add a lot of commas to your URLs, so avoid if possible.'),
          '#default_value' => intval($this->get_option('pager_element')),
        );
        break;
      case 'items_per_page':
        $form['#title'] .= $this->use_pager() ? t('Items per page') : t('Items to display');

        $form['items_per_page'] = array(
          '#type' => 'textfield',
          '#description' => t('The number of items to display per page. Enter 0 for no limit.'),
          '#default_value' => intval($this->get_option('items_per_page')),
        );
        $form['offset'] = array(
          '#type' => 'textfield',
          '#title' => t('Offset'),
          '#description' => t('The number of items to skip. For example, if this field is 3, the first 3 items will be skipped and not displayed.'),
          '#default_value' => intval($this->get_option('offset')),
        );
        break;
      case 'access':
        $form['#title'] .= t('Access restrictions');
        $form['access'] = array(
          '#prefix' => '<div class="clear-block">',
          '#suffix' => '</div>',
          '#tree' => TRUE,
        );

        $access = $this->get_option('access');
        if (empty($access)) {
          $access = array('type' => 'none', 'role' => array(), 'perm' => '');
        }
        $form['access']['type'] = array(
          '#prefix' => '<div class="views-left-25">',
          '#suffix' => '</div>',
          '#title' => t('Type'),
          '#type' => 'radios',
          '#options' => array('none' => t('Unrestricted'), 'role' => t('By role'), 'perm' => t('By perm')),
          '#default_value' => $access['type'],
        );
        $form['access']['role'] = array(
          '#prefix' => '<div class="views-left-25">',
          '#suffix' => '</div>',
          '#type' => 'checkboxes',
          '#title' => t('If by role'),
          '#default_value' => $access['role'],
          '#options' => views_ui_get_roles(),
          '#description' => t('Only the checked roles will be able to access this display.'),
        );

        $perms = array();
        // Get list of permissions
        foreach (module_list(FALSE, FALSE, TRUE) as $module) {
          if ($permissions = module_invoke($module, 'perm')) {
            $perms[$module] = drupal_map_assoc($permissions);
          }
        }
        $form['access']['perm'] = array(
          '#prefix' => '<div class="views-left-30">',
          '#suffix' => '</div>',
          '#type' => 'select',
          '#options' => $perms,
          '#title' => t('If by perm'),
          '#default_value' => $access['perm'],
          '#description' => t('Only users with the selected permission flag will be able to access this display.'),
        );
        break;
      case 'header':
        $form['#title'] .= t('Header');
        $form['header_empty'] = array(
          '#type' => 'checkbox',
          '#title' => t('Display even if view has no result'),
          '#default_value' => $this->get_option('header_empty'),
        );
        $form['header'] = array(
          '#type' => 'textarea',
          '#default_value' => $this->get_option('header'),
          '#rows' => 6,
          '#description' => t('Text to display at the top of the view. May contain an explanation or links or whatever you like. Optional.'),
        );

        $form['header_format'] = filter_form($this->get_option('header_format'), NULL, array('header_format'));
        break;
      case 'footer':
        $form['#title'] .= t('Footer');
        $form['footer_empty'] = array(
          '#type' => 'checkbox',
          '#title' => t('Display even if view has no result'),
          '#default_value' => $this->get_option('header_empty'),
        );
        $form['footer'] = array(
          '#type' => 'textarea',
          '#default_value' => $this->get_option('footer'),
          '#rows' => 6,
          '#description' => t('Text to display beneath the view. May contain an explanation or links or whatever you like. Optional.'),
        );

        $form['footer_format'] = filter_form($this->get_option('footer_format'), NULL, array('footer_format'));
        break;
      case 'empty':
        $form['#title'] .= t('Empty text');
        $form['empty'] = array(
          '#type' => 'textarea',
          '#default_value' => $this->get_option('empty'),
          '#rows' => 6,
          '#description' => t('Text to display if the view has no results. Optional.'),
        );

        $form['empty_format'] = filter_form($this->get_option('empty_format'), NULL, array('empty_format'));
        break;
      case 'style_plugin':
        $form['#title'] .= t('How should this view be styled');
        $form['style_plugin'] =  array(
          '#type' => 'radios',
          '#options' => views_fetch_plugin_names('style', 'summary', FALSE),
          '#default_value' => $this->get_option('style_plugin'),
        );
        break;
      case 'style_options':
        $form['#title'] .= t('Style options');
        $style = TRUE;
        $type = 'style_plugin';

      case 'row_options':
        // if row, $style will be empty.
        if (empty($style)) {
          $form['#title'] .= t('Row style options');
          $type = 'row_plugin';
        }
        $plugin = views_get_plugin(empty($style) ? 'row' : 'style', $this->get_option($type));
        if ($plugin) {
          $form[$form_state['section']] = array(
            '#tree' => TRUE,
          );
          $plugin->init($this->view, $this->display);
          $plugin->options_form($form[$form_state['section']], $form_state);
        }
        break;
      case 'row_plugin':
        $form['#title'] .= t('How should each row in this view be styled');
        $form['row_plugin'] =  array(
          '#type' => 'radios',
          '#options' => views_fetch_plugin_names('row', 'summary', FALSE),
          '#default_value' => $this->get_option('row_plugin'),
        );
        break;
      case 'link_display':
        $form['#title'] .= t('Which display to use for path');
        foreach ($this->view->display as $display_id => $display) {
          if ($display->handler->has_path()) {
            $options[$display_id] = $display->display_title;
          }
        }
        $form['link_display'] = array(
          '#type' => 'radios',
          '#options' => $options,
          '#description' => t('Which display to use to get this display\'s path for things like summary links, rss feed links, more links, etc.'),
          '#default_value' => $this->get_link_display(),
        );
        break;

    }
  }

  /**
   * Validate the options form.
   */
  function options_validate($form, &$form_state) {
    switch ($form_state['section']) {
      case 'style_options':
        $style = TRUE;
      case 'row_options':
        // if row, $style will be empty.
        $plugin = views_get_plugin(empty($style) ? 'row' : 'style', $this->get_option(empty($style) ? 'row_plugin' : 'style_plugin'));
        if ($plugin) {
          $plugin->init($this->view, $this->display);
          $plugin->options_validate($form[$form_state['section']], $form_state);
        }
        break;
      case 'access':
        $access = $form_state['values']['access'];
        if ($access['type'] == 'role' && !array_filter($access['role'])) {
          form_error($form, t('You must select at least one role if type is "by role"'));
        }
    }
  }

  /**
   * Perform any necessary changes to the form values prior to storage.
   * There is no need for this function to actually store the data.
   */
  function options_submit($form, &$form_state) {
    $section = $form_state['section'];
    switch ($section) {
      case 'display_title':
        $this->display->display_title = $form_state['values']['display_title'];
        break;
      case 'title':
      case 'access':
      case 'link_display':
      case 'php_arg_code':
        $this->set_option($section, $form_state['values'][$section]);
        break;
      case 'use_pager':
        $this->set_option($section, $form_state['values'][$section]);
        $this->set_option('pager_element', intval($form_state['values']['pager_element']));
        break;
      case 'items_per_page':
        $this->set_option($section, intval($form_state['values'][$section]));
        $this->set_option('offset', intval($form_state['values']['offset']));
        break;
      case 'row_plugin':
        // This if prevents resetting options to default if they don't change
        // the plugin.
        if ($this->get_option($section) != $form_state['values'][$section]) {
          $this->set_option($section, $form_state['values'][$section]);
          $plugin = views_get_plugin('row', $form_state['values'][$section]);
          $this->set_option('row_options', $plugin->options($this->display));
        }
        break;
      case 'style_plugin':
        // This if prevents resetting options to default if they don't change
        // the plugin.
        if ($this->get_option($section) != $form_state['values'][$section]) {
          $this->set_option($section, $form_state['values'][$section]);
          $plugin = views_get_plugin('style', $form_state['values'][$section]);
          $this->set_option('style_options', $plugin->options($this->display));
        }
        break;
      case 'style_options':
        $style = TRUE;
      case 'row_options':
        // if row, $style will be empty.
        $plugin = views_get_plugin(empty($style) ? 'row' : 'style', $this->get_option('style_plugin'));
        if ($plugin) {
          $plugin->init($this->view, $this->display);
          $plugin->options_submit($form[$section], $form_state);
        }
        $this->set_option($section, $form_state['values'][$section]);
        break;
      case 'header':
      case 'footer':
      case 'empty':
        $this->set_option($section, $form_state['values'][$section]);
        $this->set_option($section . '_format', $form_state['values'][$section . '_format']);
        if ($section != 'empty') {
          $this->set_option($section . '_empty', $form_state['values'][$section . '_empty']);
        }
        break;
    }
  }

  /**
   * Add an override button for a given section, allowing the user to
   * change whether this info is stored on the default display or on
   * the current display.
   */
  function add_override_button(&$form, $section) {
    if ($this->is_default_display()) {
      return;
    }

    $form['override'] = array(
      '#prefix' => '<div class="views-override clear-block">',
      '#suffix' => '</div>',
    );
    if (!empty($this->display->display_options['defaults'][$section])) {
      $form['override']['button'] = array(
        '#type' => 'submit',
        '#value' => t('Override'),
        '#submit' => array('views_ui_edit_display_form_override'),
      );
      $form['override']['markup'] = array(
        '#prefix' => '<div class="description">',
        '#value' => t('This item is currently using default values; modifying this value will modify it for all displays.'),
        '#suffix' => '</div>',
      );
    }
    else {
      $form['override']['button'] = array(
        '#type' => 'submit',
        '#value' => t('Use default'),
        '#submit' => array('views_ui_edit_display_form_override'),
      );
      $form['override']['markup'] = array(
        '#prefix' => '<div class="description">',
        '#value' => t('This item is currently overriding default values; modifying this value will modify only for this display. Reverting it will remove current values and return to default values.'),
        '#suffix' => '</div>',
      );
    }
  }

  /**
   * If override/revert was clicked, perform the proper toggle.
   */
  function options_override($form, &$form_state) {
    $section = $form_state['section'];
    $options = $this->defaultable_sections($section);
    if (!$options) {
      return;
    }

    $new_state = empty($this->display->display_options['defaults'][$section]);

    // For each option that is part of this group, fix our settings.
    foreach ($options as $option) {
      if ($new_state) {
        // Revert to defaults.
        unset($this->display->display_options[$option]);
      }
      else {
        // copy existing values into our display.
        $this->display->display_options[$option] = $this->get_option($option);
      }
      $this->display->display_options['defaults'][$option] = $new_state;
    }
  }

  /**
   * Not all display plugins will support filtering
   */
  function render_filters() { }

  /**
   * Not all display plugins will have a 'more' link
   */
  function render_more_link() { }

  /**
   * Not all display plugins will have a feed icon.
   */
  function render_feed_icon() { }

  /**
   * Render the view's title for display
   * @todo Necessary? Hm.
   */
  function render_title() { }

  function render_textarea($area) {
    $value = $this->get_option($area);
    if ($value) {
      return check_markup($value, $this->get_option($area . '_format'));
    }
  }

  /**
   * Render the header of the view.
   */
  function render_header() { return $this->render_textarea('header'); }

  /**
   * Render the footer of the view.
   */
  function render_footer() { return $this->render_textarea('footer'); }

  /**
   * Render the empty text of the view.
   */
  function render_empty() { return $this->render_textarea('empty'); }
  /**
   * If this display creates a block, implement one of these.
   */
  function hook_block($op = 'list', $delta = 0, $edit = array()) { return array(); }

  /**
   * If this display creates a page with a menu item, implement it here.
   */
  function hook_menu() { return array(); }

  /**
   * Render this display.
   */
  function render() {
    $theme = views_theme_functions('views_view', $this->view, $this->display);
    return theme($theme, $this->view);
  }

  /**
   * Determine if the user has access to this display of the view.
   */
  function access($account) {
    $access = $this->get_option('access');
    switch ($access['type']) {
      case 'role':
        $roles = array_keys($account->roles);
        $roles[] = $account->uid ? DRUPAL_AUTHENTICATED_RID : DRUPAL_ANONYMOUS_RID;
        return array_intersect(array_filter($access['role']), $roles);
      case 'perm':
        return user_access($access['perm'], $account);
      case 'none':
      default:
        return TRUE;
    }
  }
  /**
   * When used externally, this is how a view gets run and returns
   * data in the format required.
   *
   * The base class cannot be executed.
   */
  function execute() { }
}

/**
 * A plugin to handle defaults on a view.
 */
class views_display_plugin_default extends views_display_plugin {
  /**
   * Determine if this display is the 'default' display which contains
   * fallback settings
   */
  function is_default_display() { return TRUE; }

  function options(&$display) {
    // Make sure the default display has a style plugin to start with.
    $display->display_options['style_plugin'] = 'default';
    $display->display_options['style_options'] = array();
    $display->display_options['row_plugin'] = 'fields';
    $display->display_options['row_options'] = array();
    $display->display_options['relationships'] = array();
    $display->display_options['fields'] = array();
    $display->display_options['sorts'] = array();
    $display->display_options['arguments'] = array();
    $display->display_options['filters'] = array();

    $display->display_options['items_per_page'] = 10;
  }

  /**
   * The default execute handler fully renders the view.
   *
   * For the simplest use:
   * @code
   *   $output = $view->execute_display('default', $args);
   * @endcode
   *
   * For more complex usages, a view can be partially built:
   * @code
   *   $view->set_arguments($args);
   *   $view->build('default'); // Build the query
   *   $view->execute(); // Run the query
   *   $output = $view->render(); // Render the view
   * @endcode
   *
   * If short circuited at any point, look in $view->build_info for
   * information about the query. After execute, look in $view->result
   * for the object returned from db_query.
   *
   * You can also do:
   * @code
   *   $view->set_arguments($args);
   *   $output = $view->render('default'); // Render the view
   * @endcode
   *
   * This illustrates that render is smart enough to call build and execute
   * if these items have not already been accomplished.
   *
   * Note that execute also must accomplish other tasks, such
   * as setting page titles, breadcrumbs, and generating exposed filter
   * data if necessary.
   */
  function execute() {
    return $this->view->render();
  }
}

/**
 * The plugin that handles a full page.
 */
class views_display_plugin_page extends views_display_plugin {
  /**
   * The page display has a path.
   */
  function has_path() { return TRUE; }
  function uses_breadcrumb() { return TRUE; }

  /**
   * Add this display's path information to Drupal's menu system.
   */
  function execute_hook_menu() {
    $items = array();
    // Replace % with the link to our standard views argument loader
    // views_arg_load -- which lives in views.module

    $bits = explode('/', $this->get_option('path'));
    $page_arguments = array($this->view->name, $this->display->id);

    // Replace % with %views_arg for menu autoloading and add to the
    // page arguments so the argument actually comes through.
    foreach($bits as $pos => $bit) {
      if ($bit == '%') {
        $bits[$pos] = '%views_arg';
        $page_arguments[] = $pos;
      }
    }

    $path = implode('/', $bits);

    if ($path) {
      // NOTE: This is the very simple 'menu normal item' version. The
      // tab version needs to come later. Maybe it should be its own plugin.
      $items[$path] = array(
        // default views page entry
        'page callback' => 'views_page',
        'page arguments' => $page_arguments,
        // Default access check (per display)
        'access callback' => 'views_access',
        'access arguments' => array(array($this->view->name, $this->display->id)),
        // Identify URL embedded arguments and correlate them to a handler
        'load arguments'  => array($this->view->name, $this->display->id, '%index'),
        // Basic menu title
        'title' => $this->display->handler->get_option('title'),
        'type' => MENU_NORMAL_ITEM,
      );
    }

    return $items;
  }

  /**
   * The display page handler returns a normal view, but it also does
   * a drupal_set_title for the page, and does a views_set_page_view
   * on the view.
   */
  function execute() {
    // Let the world know that this is the page view we're using.
    views_set_page_view($this);

    // Prior to this being called, the $view should already be set to this
    // display, and arguments should be set on the view.
    $this->view->build();
    $this->view->get_breadcrumb(TRUE);

    // And the title, which is much easier.
    drupal_set_title(filter_xss_admin($this->view->get_title()));

    // And now render the view.
    return $this->view->render();
  }

  /**
   * Provide the summary for page options in the views UI.
   *
   * This output is returned as an array.
   */
  function options_summary(&$categories, &$options) {
    // It is very important to call the parent function here:
    parent::options_summary($categories, $options);

    $categories['page'] = array(
      'title' => t('Page settings'),
    );

    $path = $this->get_option('path');
    if (empty($path)) {
      $path = t('None');
    }

    if (strlen($path) > 16) {
      $path = substr($path, 0, 16) . '...';
    }

    $options['path'] = array(
      'category' => 'page',
      'title' => t('Path'),
      'value' => $path,
    );

    $options['menu'] = array(
      'category' => 'page',
      'title' => t('Menu'),
      'value' => 'TODO',
    );
  }

  /**
   * Provide the default form for setting options.
   */
  function options_form(&$form, &$form_state) {
    // It is very important to call the parent function here:
    parent::options_form($form, $form_state);

    switch ($form_state['section']) {
      case 'path':
        $form['#title'] .= t('The menu path or URL of this view');
        $form['path'] = array(
          '#type' => 'textfield',
          '#description' => t('This view will be displayed by visiting this path on your site.'),
          '#default_value' => $this->get_option('path'),
        );
        break;
    }
  }

  /**
   * Validate the options form.
   */
  function options_validate($form, &$form_state) {
    // It is very important to call the parent function here:
    parent::options_validate($form, $form_state);

    switch ($form_state['section']) {
      case 'path':
        // @todo: validate the path against other views

        // @todo: validate the path against aliases.
        break;
    }
  }

  /**
   * Perform any necessary changes to the form values prior to storage.
   * There is no need for this function to actually store the data.
   */
  function options_submit($form, &$form_state) {
    // It is very important to call the parent function here:
    parent::options_submit($form, $form_state);
    switch ($form_state['section']) {
      case 'path':
        $this->set_option('path', $form_state['values']['path']);
        break;
    }
  }
}

/**
 * The plugin that handles a block.
 */
class views_display_plugin_block extends views_display_plugin {
  /**
   * The default block handler doesn't support configurable items,
   * but extended block handlers might be able to do interesting
   * stuff with it.
   */
  function execute_hook_block($op = 'list', $delta = 0, $edit = array()) {
    if ($op == 'list') {
      $delta = $this->view->name . '-' . $this->display->id;
      $desc = $this->get_option('block_description');

      if (empty($desc)) {
        $desc = $this->view->name;
      }
      return array($delta => array('info' => $desc));
    }
  }

  /**
   * The display block handler returns the structure necessary for a block.
   */
  function execute() {
    // Prior to this being called, the $view should already be set to this
    // display, and arguments should be set on the view.
    $info['content'] = $this->view->render();
    $info['subject'] = filter_xss_admin($this->view->get_title());
    return $info;
  }

  /**
   * Provide the summary for page options in the views UI.
   *
   * This output is returned as an array.
   */
  function options_summary(&$categories, &$options) {
    // It is very important to call the parent function here:
    parent::options_summary($categories, $options);

    $categories['block'] = array(
      'title' => t('Block settings'),
    );

    $block_description = $this->get_option('block_description');
    if (empty($block_description)) {
      $block_description = t('None');
    }

    if (strlen($block_description) > 16) {
      $block_description = substr($block_description, 0, 16) . '...';
    }

    $options['block_description'] = array(
      'category' => 'block',
      'title' => t('Admin'),
      'value' => $block_description,
    );
  }

  /**
   * Provide the default form for setting options.
   */
  function options_form(&$form, &$form_state) {
    // It is very important to call the parent function here:
    parent::options_form($form, $form_state);

    switch ($form_state['section']) {
      case 'block_description':
        $form['#title'] .= t('Block admin description');
        $form['block_description'] = array(
          '#type' => 'textfield',
          '#description' => t('This will appear as the name of this block in administer >> site building >> blocks.'),
          '#default_value' => $this->get_option('block_description'),
        );
        break;
    }
  }

  /**
   * Validate the options form.
   */
  function options_validate($form, &$form_state) {
    // It is very important to call the parent function here:
    parent::options_validate($form, $form_state);

    switch ($form_state['section']) {
      case 'block_description':
        // @todo: validate the block_description against other views

        // @todo: validate the block_description against aliases.
        break;
    }
  }

  /**
   * Perform any necessary changes to the form values prior to storage.
   * There is no need for this function to actually store the data.
   */
  function options_submit($form, &$form_state) {
    // It is very important to call the parent function here:
    parent::options_submit($form, $form_state);
    switch ($form_state['section']) {
      case 'block_description':
        $this->set_option('block_description', $form_state['values']['block_description']);
        break;
    }
  }

  /**
   * Block views do not use exposed widgets.
   */
  function uses_exposed() { return FALSE; }
}

/**
 * @}
 */

/**
 * @defgroup views_style_plugins Views' style plugins
 * @{
 * Style plugins control how a view is rendered. For example, they
 * can choose to display a collection of fields, node_view() output,
 * table output, or any kind of crazy output they want.
 *
 * Many style plugins can have an optional 'row' plugin, that displays
 * a single record. Not all style plugins can utilize this, so it is
 * up to the plugin to set this up and call through to the row plugin.
 */

/**
 * Base class to define a style plugin handler.
 */
class views_style_plugin extends views_object {
  /**
   * Initialize a style plugin.
   *
   * @param $view
   * @param $display
   * @param $options
   *   The style options might come externally as the style can be sourced
   *   from at least two locations. If it's not included, look on the display.
   */
  function init(&$view, &$display, $options = NULL) {
    $this->view = $view;
    $this->display = $display;
    if (isset($options)) {
      $this->options = $options;
    }
    else {
      $this->options = $display->handler->get_option('style_options');
    }

    if ($this->uses_row_plugin()) {
      $this->row_plugin = views_get_plugin('row', $display->handler->get_option('row_plugin'));
      // initialize the row plugin.
      if ($this->row_plugin) {
        $this->row_plugin->init($view, $display);
      }
    }
  }

  /**
   * Return TRUE if this style also uses a row plugin.
   */
  function uses_row_plugin() {
    return !empty($this->definition['uses row plugin']);
  }

  /**
   * Return TRUE if this style also uses fields.
   */
  function uses_fields() {
    // If we use a row plugin, ask the row plugin. Chances are, we don't
    // care, it does.
    if ($this->uses_row_plugin()) {
      return $this->row_plugin->uses_fields();
    }
    // Otherwise, maybe we do.
    return !empty($this->definition['uses fields']);
  }

  /**
   * Static member function to set default options.
   */
  function options(&$display) {
    return array();
  }

  /**
   * Provide a form for setting options.
   */
  function options_form(&$form, &$form_state) { }

  /**
   * Validate the options form.
   */
  function options_validate($form, &$form_state) { }

  /**
   * Perform any necessary changes to the form values prior to storage.
   * There is no need for this function to actually store the data.
   */
  function options_submit($form, &$form_state) { }

  function render($rows) { }
}

/**
 * Default style plugin to render rows one after another with no
 * decorations.
 */
class views_style_plugin_default extends views_style_plugin {
  /**
   * Render the given style.
   */
  function render() {
    if (empty($this->row_plugin)) {
      vpr('views_style_plugin_default: Missing row plugin');
      return;
    }
    // @todo: This needs to be able to support either a database resource OR
    // an array, because our input format doesn't actually have to be from
    // a query.
    $rows = array();
    while ($row = db_fetch_object($this->view->result)) {
      // @todo: Include separator as an option.
      $rows[] = $this->row_plugin->render($row);
    }
    return theme(array('views_view_unformatted__' . $this->view->name, 'views_view_unformatted'), $this->view, $this->options, $rows);
  }
}

/**
 * Style plugin to render each item in an ordered or unordered list
 */
class views_style_plugin_list extends views_style_plugin {
  /**
   * Set default options
   */
  function options($display) {
    return array(
      'type' => 'ul',
    );
  }

  /**
   * Render the given style.
   */
  function options_form(&$form, &$form_state) {
    $form['type'] = array(
      '#type' => 'radios',
      '#title' => t('List type'),
      '#options' => array('ul' => t('Unordered list'), 'ol' => t('Ordered list')),
      '#default_value' => $this->options['type'],
    );
  }

  function render() {
    if (empty($this->row_plugin)) {
      // @todo: Log some form of error here?
      return;
    }
    // @todo: This needs to be able to support either a database resource OR
    // an array, because our input format doesn't actually have to be from
    // a query.
    $rows = array();
    while ($row = db_fetch_object($this->view->result)) {
      $rows[] = $this->row_plugin->render($row);
    }
    return theme(array('views_view_list__' . $this->view->name, 'views_view_list'), $this->view, $this->options, $rows);
  }
}

/**
 * Style plugin to render each item as a row in a table.
 */
class views_style_plugin_table extends views_style_plugin {
  /**
   * Set default options
   */
  function options($display) {
    return array(
    );
  }

  /**
   * Render the given style.
   */
  function options_form(&$form, &$form_state) {
    $form['markup'] = array(
      '#default_value' => 'ToDo',
    );
  }

  function render() {
    if (empty($this->row_plugin)) {
      // @todo: Log some form of error here?
      return;
    }
    // @todo: This needs to be able to support either a database resource OR
    // an array, because our input format doesn't actually have to be from
    // a query.
    $rows = '';
    return "Table not yet implemented";
  }
}

/**
 * The default style plugin for summaries.
 */
class views_style_plugin_summary extends views_style_plugin {
  function options_form(&$form, &$form_state) {
    $form['count'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['count']),
      '#title' => t('Display record count with link'),
    );
  }

  /**
   * Render the given style.
   */
  function render() {
    // @todo: This needs to be able to support either a database resource OR
    // an array, because our input format doesn't actually have to be from
    // a query.
    $rows = array();
    while ($row = db_fetch_object($this->view->result)) {
      // @todo: Include separator as an option.
        $rows[] = $row;
    }
    return theme(array('views_view_summary__' . $this->view->name, 'views_view_summary'), $this->view, $this->options, $rows);
  }
}

/**
 * @}
 */

/**
 * @defgroup views_row_plugins Views' row plugins
 * @{
 *
 * Row plugins control how Views outputs an individual record. They are
 * tightly coupled to style plugins, in that a style plugin is what calls
 * the row plugin.
 */

/**
 * Default plugin to view a single row of a table. This is really just a wrapper around
 * a theme function.
 */
class views_row_plugin extends views_object {
  /**
   * Initialize the row plugin.
   */
  function init(&$view, &$display) {
    $this->view = $view;
    $this->display = $display;
    $this->options = $display->handler->get_option('row_options');
  }

  function uses_fields() {
    return !empty($this->definition['uses fields']);
  }

  /**
   * Static member function to set default options.
   */
  function options(&$display) {
    return array();
  }

  /**
   * Provide a form for setting options.
   */
  function options_form(&$form, &$form_state) { }

  /**
   * Validate the options form.
   */
  function options_validate($form, &$form_state) { }

  /**
   * Perform any necessary changes to the form values prior to storage.
   * There is no need for this function to actually store the data.
   */
  function options_submit($form, &$form_state) { }

  /**
   * Render a row object. This usually passes through to a theme template
   * of some form, but not always.
   */
  function render($row) {
    return theme(array('views_view_fields__' . $this->view->name, 'views_view_fields'), $this->view, $this->options, $row);
  }
}

/**
 * @}
 */

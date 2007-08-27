<?php 
// $Id$
/**
 * @file Built in plugins for Views output handling.
 *
 */

/** 
 * Implementation of hook_views_plugins
 */
function views_views_plugins() {
  return array(
    'module' => 'views', // This just tells our themes are elsewhere.
    'display' => array(
      'page' => array(
        'title' => t('Page'),
        'help' => t('Creates a page with a URL, menu links, etc.'),
        'handler' => 'views_display_plugin_page',
      ),
      'block' => array(
         'title' => t('Block'),
         'help' => t('Creates a block that can be used from the block administration page.'),
         'handler' => 'views_display_plugin_block',
      ),
      'embed' => array(
        'title' => t('Embedded'),
        'help' => t('Creates a view that is used from other code. By itself an embedded view does not do anything.'),
        'handler' => 'views_display_plugin',
      ),
    ),
    'style' => array(
      'default' => array(
        'title' => t('Default'),
        'help' => t('Displays rows one after another.'),
        'handler' => 'views_style_plugin_default',
        'theme' => 'views_view_rows',
      ),
      'list' => array(
        'title' => t('List'),
        'help' => t('Displays rows as an HTML list.'),
        'handler' => 'views_style_plugin_list',
        'theme' => 'views_view_list',
      ),
      'table' => array(
        'title' => t('Table'),
        'help' => t('Displays rows in a table.'),
        'handler' => 'views_style_plugin_table',
        'theme' => 'views_view_table',
      ),
    ),
    'row' => array(
      'fields' => array(
        'title' => t('Fields'),
        'help' => t('Displays the fields with an optional template.'),
        'handler' => 'views_row_plugin',
        'theme' => 'views_view_row',
      ),
    ),
  );

}

/**
 * The default display plugin handler. Display plugins handle options and
 * basic mechanisms for different output methods.
 *
 * -- hook_menu
 * -- hook_menu
 * -- render_page
 */
class views_display_plugin extends views_object {
  var $uses_hook_block = FALSE;
  var $uses_hook_menu = FALSE;

  /**
   * Fill this plugin in with the view, display, etc.
   */
  function seed(&$view, $display) {
    $this->view = $view;
    $this->display = $display;
  }

  /**
   * Intelligently get an option either from this display or from the
   * default display, if directed to do so.
   */
  function get_option($option) {
    if (isset($this->display->display_options[$option])) {
      return $this->display->display_options[$option];
    }
    if (empty($this->default_display)) {
      return;
    }
    if (empty($this->display_options[$option . '_default'])) {
      return;      
    }
    if (isset($this->default_display->display_options[$option])) {
      return $this->default_display->display_options[$option];
    }
  }  

  /**
   * Provide the default form for setting options.
   */
  function options_form(&$form) { }
  
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
   * Not all display plugins will support filtering
   */
  function render_filters() { }

  /**
   * Not all display plugins will have a 'more' link
   */
  function render_more_link() { }

  /**
   * Not all display plugins will have a feed icon, nor will they
   * put it in the same place.
   */ 
  function render_feed_icon() { }

  /**
   * Render the view's title for display
   */
  function render_title() { }

  function render_header() { }
  function render_footer() { }
  function render_empty() { return 'empty text'; }
  /**
   * If this display creates a block, implement one of these.
   */
//  function hook_block($op = 'list', $delta = 0, $edit = array()) {}

  /**
   * If this display creates a page with a menu item, implement it here.
   */
//  function hook_menu() {}

  /**
   * Render this display.
   */
  function render() {
    // TODO: Remove this when the 'file' tag on theme registry is fixed.
    include_once drupal_get_path('module', 'views') . '/theme/theme.inc';
    $themes = array(
      'views_view__' . $this->display->id . '__' . $this->view->name,
      'views_view__' . $this->display->id,
      'views_view__' . $this->display->display_plugin . '__' . $this->view->name,
      'views_view__' . $this->display->display_plugin,
      'views_view__' . $this->view->name,
      'views_view',
    );
    return theme($themes, $this->view);
  }

}

/**
 * The plugin that handles a full page.
 */
class views_display_plugin_page extends views_display_plugin {
  var $uses_hook_menu = TRUE;

  function hook_menu() {}

  function render_page() {}
}

/**
 * The plugin that handles a block.
 */
class views_display_plugin_block extends views_display_plugin {
  var $uses_hook_block = TRUE;
  /**
   * If this display creates a block, implement one of these.
   */
  function hook_block($op = 'list', $delta = 0, $edit = array()) {}
}

/**
 * Base class to define a style plugin handler.
 */
class views_style_plugin extends views_object {
  var $needs_fields = FALSE;
  var $needs_headers = FALSE;

  function seed(&$view, $display) {
    $this->view = $view;
    $this->display = $display;
    $this->options = $display->style_options;
  }

  /**
   * Provide a form for setting options.
   */
  function options_form(&$form) { }
  
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

class views_style_plugin_default extends views_style_plugin {
  // TEMP HACK
  var $row_plugin = 'views_row_plugin';
  function options_form(&$form) {
    // provide an option form to select from our list of node renderers
  }

  function render() {
    $plugin = new views_row_plugin;
    $rows = '';
    while ($row = db_fetch_object($this->view->result)) {
      $rows .= $plugin->render($this->view, $row);
    }
    return theme(array('views_view_rows__' . $this->view->name, 'views_view_rows'), $this->view, $rows);
  }
}

/**
 * Default plugin to view a single row of a table. This is really just a wrapper around
 * a theme function.
 */
class views_row_plugin extends views_object {
  function render(&$view, $row) {
    return theme(array('views_view_row__' . $view->name, 'views_view_row'), $view, $row);
  }
}

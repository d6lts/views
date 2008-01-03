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
      'page' => array(
        'title' => t('Page'),
        'help' => t('Creates a page with a URL, menu links, etc.'),
        'handler' => 'views_display_plugin_page',
        'uses_hook_menu' => TRUE,
      ),
      'block' => array(
        'title' => t('Block'),
        'help' => t('Creates a block that can be used from the block administration page.'),
        'handler' => 'views_display_plugin_block',
        'uses_hook_block' => TRUE,
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
        'theme' => 'views_view_summary',
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
      'default_summary' => array(
        'summary' => TRUE, // only shows up as a summary style
        'title' => t('Default'),
        'help' => t('Displays the default summary view'),
        'handler' => 'views_style_plugin_summary',
        'theme' => 'views_view_summary',
      ),
    ),
    'row' => array(
      'fields' => array(
        'title' => t('Fields'),
        'help' => t('Displays the fields with an optional template.'),
        'handler' => 'views_row_plugin',
        'theme' => 'views_view_row',
      ),
      'fields_summary' => array(
        'title' => t('Summary'),
        'help' => t('Displays the link to the argument and the row count.'),
        'handler' => 'views_row_plugin_summary',
        'theme' => 'views_view_row_summary',
      ),
    ),
  );

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
  function init(&$view, &$display) {
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
   * Not all display plugins will have a feed icon.
   */
  function render_feed_icon() { }

  /**
   * Render the view's title for display
   * @todo: Necessary? Hm.
   */
  function render_title() { }

  function render_textarea($area) {
    $format_string = $area . '_format';
    return check_markup($this->display->$area, $this->display->$format_string);
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

  /**
   * Determine if the user has access to this display of the view.
   *
   * @todo Implement this.
   */
  function access($account) { return TRUE; }
  /**
   * When used externally, this is how a view gets run and returns
   * data in the format required.
   */
  function execute() { }
}

/**
 * The plugin that handles a full page.
 */
class views_display_plugin_page extends views_display_plugin {
  var $uses_hook_menu = TRUE;

  function execute_hook_menu() {
    $items = array();
    // Replace % with the link to our standard views argument loader
    // views_arg_load -- which lives in views.module
    $url = str_replace('%', '%views_arg', $this->display->url);

    // NOTE: This is the very simple 'menu normal item' version. The
    // tab version needs to come later. Maybe it should be its own plugin.
    $items[$url] = array(
      // default views page entry
      'page callback' => 'views_page',
      'page arguments' => array($this->view->name, $this->display->id),
      // Default access check (per display)
      'access callback' => 'views_access',
      'access arguments' => array(array($this->view->name, $this->display->id)),
      // Identify URL embedded arguments and correlate them to a handler
      'load arguments'  => array($this->view->name, '%index'),
      // Basic menu title
      'title' => $this->display->title,
      'type' => MENU_NORMAL_ITEM,
    );
    return $items;
  }

  /**
   * The display page handler returns a normal view, but it also does
   * a drupal_set_title for the page, and does a views_set_page_view
   * on the view.
   */
  function execute() {
    views_set_page_view($this);
    // Prior to this being called, the $view should already be set to this
    // display, and arguments should be set on the view.
    $this->view->build();
    drupal_set_title(filter_xss_admin($this->view->get_title('page')));
    return $this->view->render();
  }
}

/**
 * The plugin that handles a block.
 */
class views_display_plugin_block extends views_display_plugin {
  var $uses_hook_block = TRUE;

  /**
   * The default block handler doesn't support configurable items,
   * but extended block handlers might be able to do interesting
   * stuff with it.
   */
  function hook_block($op = 'list', $delta = 0, $edit = array()) {
    if ($op == 'list') {
      $delta = $this->view->name . '-' . $this->display->id;
      return array($delta => array('info' => $this->display->block_description));
    }
  }

  /**
   * The display block handler returns the structure necessary for a block.
   */
  function execute() {
    drupal_set_page_view($this);
    // Prior to this being called, the $view should already be set to this
    // display, and arguments should be set on the view.
    $info['content'] = $this->view->render();
    $info['subject'] = filter_xss_admin($this->view->get_title('page'));
    return $info;
  }
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
  var $needs_fields = FALSE;
  var $needs_headers = FALSE;

  function init(&$view, &$display) {
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

/**
 * Default style plugin to render rows one after another with no
 * decorations.
 */
class views_style_plugin_default extends views_style_plugin {
  // TEMP HACK
  var $row_plugin = 'views_row_plugin';
//  var $row_plugin = 'views_row_plugin_node_view';
  function options_form(&$form) {
    // provide an option form to select from our list of node renderers
  }

  /**
   * Render the given style.
   */
  function render() {
    // @todo: This needs to be able to support either a database resource OR
    // an array, because our input format doesn't actually have to be from
    // a query.
    $plugin = new $this->row_plugin;
    $rows = '';
    while ($row = db_fetch_object($this->view->result)) {
      // @todo: Include separator as an option.
      $rows .= $plugin->render($this->view, $row);
    }
    return theme(array('views_view_rows__' . $this->view->name, 'views_view_rows'), $this->view, $rows);
  }
}

/**
 * The default style plugin for summaries.
 */
class views_style_plugin_summary extends views_style_plugin {
  // TEMP HACK
  var $row_plugin = 'views_row_plugin_summary';
  function options_form(&$form) {
    // provide an option form to select from our list of node renderers
  }

  /**
   * Render the given style.
   */
  function render() {
    // @todo: This needs to be able to support either a database resource OR
    // an array, because our input format doesn't actually have to be from
    // a query.
    $plugin = new $this->row_plugin;
    $rows = array();
    while ($row = db_fetch_object($this->view->result)) {
      // @todo: Include separator as an option.
      $rows[] = $plugin->render($this->view, $row);
    }
    return theme(array('views_view_summary__' . $this->view->name, 'views_view_summary'), $this->view, $rows);
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
  function render(&$view, $row) {
    return theme(array('views_view_row__' . $view->name, 'views_view_row'), $view, $row);
  }
}

/**
 * Default plugin to view a single row of a table. This is really just a wrapper around
 * a theme function.
 */
class views_row_plugin_summary extends views_object {
  function render(&$view, $row) {
    return theme(array('views_view_row_summary__' . $view->name, 'views_view_row_summary'), $view, $row);
  }
}

/**
 * @}
 */

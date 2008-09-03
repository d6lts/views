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
  $path = drupal_get_path('module', 'views') . '/js';
  return array(
    'module' => 'views', // This just tells our themes are elsewhere.
    'display' => array(
      'parent' => array(
        // this isn't really a display but is necessary so the file can
        // be included.
        'no ui' => TRUE,
        'handler' => 'views_plugin_display',
        'parent' => '',
      ),
      'default' => array(
        'title' => t('Defaults'),
        'help' => t('Default settings for this view.'),
        'handler' => 'views_plugin_display_default',
        'theme' => 'views_view',
        'no ui' => TRUE,
        'no remove' => TRUE,
        'js' => array('misc/collapse.js', 'misc/textarea.js', 'misc/tabledrag.js', 'misc/autocomplete.js', "$path/dependent.js"),
        'use ajax' => TRUE,
        'use pager' => TRUE,
        'use more' => TRUE,
        'accept attachments' => TRUE,
        'help topic' => 'display-default',
      ),
      'page' => array(
        'title' => t('Page'),
        'help' => t('Display the view as a page, with a URL and menu links.'),
        'handler' => 'views_plugin_display_page',
        'theme' => 'views_view',
        'uses hook menu' => TRUE,
        'use ajax' => TRUE,
        'use pager' => TRUE,
        'accept attachments' => TRUE,
        'admin' => t('Page'),
        'help topic' => 'display-page',
      ),
      'block' => array(
        'title' => t('Block'),
        'help' => t('Display the view as a block.'),
        'handler' => 'views_plugin_display_block',
        'theme' => 'views_view',
        'uses hook block' => TRUE,
        'use ajax' => TRUE,
        'use pager' => TRUE,
        'use more' => TRUE,
        'accept attachments' => TRUE,
        'admin' => t('Block'),
        'help topic' => 'display-block',
      ),
      'attachment' => array(
        'title' => t('Attachment'),
        'help' => t('Attachments added to other displays to achieve multiple views in the same view.'),
        'handler' => 'views_plugin_display_attachment',
        'theme' => 'views_view',
        'use ajax' => TRUE,
        'help topic' => 'display-attachment',
      ),
      'feed' => array(
        'title' => t('Feed'),
        'help' => t('Display the view as a feed, such as an RSS feed.'),
        'handler' => 'views_plugin_display_feed',
        'parent' => 'page', // so it knows to load the page plugin .inc file
        'uses hook menu' => TRUE,
        'use ajax' => FALSE,
        'use pager' => FALSE,
        'accept attachments' => FALSE,
        'admin' => t('Feed'),
        'help topic' => 'display-feed',
      ),
    ),
    'style' => array(
      'parent' => array(
        // this isn't really a display but is necessary so the file can
        // be included.
        'no ui' => TRUE,
        'handler' => 'views_plugin_style',
        'parent' => '',
      ),
      'default' => array(
        'title' => t('Unformatted'),
        'help' => t('Displays rows one after another.'),
        'handler' => 'views_plugin_style_default',
        'theme' => 'views_view_unformatted',
        'uses row plugin' => TRUE,
        'uses options' => TRUE,
        'uses grouping' => TRUE,
        'type' => 'normal',
      ),
      'list' => array(
        'title' => t('List'),
        'help' => t('Displays rows as an HTML list.'),
        'handler' => 'views_plugin_style_list',
        'theme' => 'views_view_list',
        'uses row plugin' => TRUE,
        'uses options' => TRUE,
        'type' => 'normal',
      ),
      'grid' => array(
        'title' => t('Grid'),
        'help' => t('Displays rows in a grid.'),
        'handler' => 'views_plugin_style_grid',
        'theme' => 'views_view_grid',
        'uses row plugin' => TRUE,
        'uses options' => TRUE,
        'type' => 'normal',
      ),
      'table' => array(
        'title' => t('Table'),
        'help' => t('Displays rows in a table.'),
        'handler' => 'views_plugin_style_table',
        'theme' => 'views_view_table',
        'uses row plugin' => FALSE,
        'uses fields' => TRUE,
        'uses options' => TRUE,
        'type' => 'normal',
      ),
      'default_summary' => array(
        'title' => t('List'),
        'help' => t('Displays the default summary summary as a list.'),
        'handler' => 'views_plugin_style_summary',
        'theme' => 'views_view_summary',
        'type' => 'summary', // only shows up as a summary style
        'uses options' => TRUE,
      ),
      'unformatted_summary' => array(
        'title' => t('Unformatted'),
        'help' => t('Displays the summary unformatted, with option for one after another or inline.'),
        'handler' => 'views_plugin_style_summary_unformatted',
        'parent' => 'default_summary',
        'theme' => 'views_view_summary_unformatted',
        'type' => 'summary', // only shows up as a summary style
        'uses options' => TRUE,
      ),
      'rss' => array(
        'title' => t('RSS Feed'),
        'help' => t('Generates an RSS feed from a view.'),
        'handler' => 'views_plugin_style_rss',
        'theme' => 'views_view_rss',
        'uses row plugin' => TRUE,
        'uses options' => TRUE,
        'type' => 'feed',
      ),
    ),
    'row' => array(
      'parent' => array(
        // this isn't really a display but is necessary so the file can
        // be included.
        'no ui' => TRUE,
        'handler' => 'views_plugin_row',
        'parent' => '',
      ),
      'fields' => array(
        'title' => t('Fields'),
        'help' => t('Displays the fields with an optional template.'),
        'handler' => 'views_plugin_row_fields',
        'theme' => 'views_view_fields',
        'uses fields' => TRUE,
        'uses options' => TRUE,
        'type' => 'normal',
      ),
    ),
    'argument default' => array(
      // This type of plugin does not conform to the standard and
      // uses 'fixed' as the parent rather than having a separate parent.
      'fixed' => array(
        'title' => t('Fixed entry'),
        'handler' => 'views_plugin_argument_default',
      ),
      'php' => array(
        'title' => t('PHP Code'),
        'handler' => 'views_plugin_argument_default_php',
        'parent' => 'fixed',
      ),
    ),
    'argument validator' => array(
      'parent' => array(
        // this isn't really a display but is necessary so the file can
        // be included.
        'no ui' => TRUE,
        'handler' => 'views_plugin_argument_defaultxx',
        'parent' => '',
      ),
      'php' => array(
        'title' => t('PHP Code'),
        'handler' => 'views_plugin_argument_validate_php',
      ),
      'numeric' => array(
        'title' => t('Numeric'),
        'handler' => 'views_plugin_argument_validate_numeric',
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
      $theme_path = drupal_get_path('module', $module_dir) . '/theme';
      $theme_file = 'theme.inc';
      $path = drupal_get_path('module', $module_dir) . '/plugins';
    }
    else {
      $theme_path = $path = drupal_get_path('module', $module_dir);
      $theme_file = "$module.views.inc";
    }

    foreach ($result as $type => $info) {
      if ($type == 'module') {
        continue;
      }
      foreach ($info as $plugin => $def) {
        $def['module'] = $module_dir;
        if (!isset($def['theme path'])) {
          $def['theme path'] = $theme_path;
        }
        if (!isset($def['theme file'])) {
          $def['theme file'] = $theme_file;
        }
        if (!isset($def['path'])) {
          $def['path'] = $path;
        }
        if (!isset($def['file'])) {
          $def['file'] = $def['handler'] . '.inc';
        }
        if (!isset($def['parent'])) {
          $def['parent'] = 'parent';
        }
        // merge the new data in
        $cache[$type][$plugin] = $def;
      }
    }
  }
  return $cache;
}

/**
 * Abstract base class to provide interface common to all plugins.
 */
class views_plugin extends views_object {
  /**
   * Init will be called after construct, when the plugin is attached to a
   * view and a display.
   */
  function init(&$view, &$display) {
    $this->view = &$view;
    $this->display = &$display;
  }

  /**
   * Provide a form to edit options for this plugin.
   */
  function options_form(&$form, &$form_state) { }

  /**
   * Validate the options form.
   */
  function options_validate(&$form, &$form_state) { }

  /**
   * Handle any special handling on the validate form.
   */
  function options_submit(&$form, &$form_state) { }

  /**
   * Add anything to the query that we might need to.
   */
  function query() { }

  /**
   * Provide a full list of possible theme templates used by this style.
   */
  function theme_functions() {
    return views_theme_functions($this->definition['theme'], $this->view, $this->display);
  }

  /**
   * Provide a list of additional theme functions for the theme information page
   */
  function additional_theme_functions() {
    $funcs = array();
    if (!empty($this->definition['additional themes'])) {
      foreach ($this->definition['additional themes'] as $theme) {
        $funcs[] = views_theme_functions($theme, $this->view, $this->display);
      }
    }
    return $funcs;
  }

  /**
   * Validate that the plugin is correct and can be saved.
   *
   * @return
   *   An array of error strings to tell the user what is wrong with this
   *   plugin.
   */
  function validate() { return array(); }
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
 *
 * @see hook_views_plugins
 */

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
 *
 * @see hook_views_plugins
 */

/**
 * Base class to define a style plugin handler.
 */
class views_plugin_style extends views_plugin {
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
    $this->view = &$view;
    $this->display = &$display;

    // Overlay incoming options on top of defaults
    $this->unpack_options($this->options, isset($options) ? $options : $display->handler->get_option('style_options'));

    if ($this->uses_row_plugin() && $display->handler->get_option('row_plugin')) {
      $this->row_plugin = $display->handler->get_plugin('row');
    }

    $this->options += array(
      'grouping' => '',
    );

    $this->definition += array(
      'uses grouping' => TRUE,
    );
  }

  function destroy() {
    parent::destroy();

    if (isset($this->row_plugin)) {
      $this->row_plugin->destroy();
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
    if ($this->uses_row_plugin() && !empty($this->row_plugin)) {
      return $this->row_plugin->uses_fields();
    }
    // Otherwise, maybe we do.
    return !empty($this->definition['uses fields']);
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['grouping'] = array('default' => '');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    // Only fields-based views can handle grouping.  Style plugins can also exclude
    // themselves from being groupable by setting their "use grouping" definiton
    // key to FALSE.
    // @TODO: Document "uses grouping" in docs.php when docs.php is written.
    if ($this->uses_fields() && $this->definition['uses grouping']) {
      $options = array('' => t('<None>'));
      foreach ($this->display->handler->get_handlers('field') as $field => $handler) {

        if ($label = $handler->label()) {
          $options[$field] = $label;
        }
        else {
          $options[$field] = $handler->ui_name();
        }
      }

      // If there are no fields, we can't group on them.
      if (count($options) > 1) {
        $form['grouping'] = array(
          '#type' => 'select',
          '#title' => t('Grouping field'),
          '#options' => $options,
          '#default_value' => $this->options['grouping'],
          '#description' => t('You may optionally specify a field by which to group the records.  Leave blank to not group.'),
        );
      }
    }
  }

  /**
   * Called by the view builder to see if this style handler wants to
   * interfere with the sorts. If so it should build; if it returns
   * any non-TRUE value, normal sorting will NOT be added to the query.
   */
  function build_sort() { return TRUE; }

  /**
   * Allow the style to do stuff before each row is rendered.
   *
   * @param $result
   *   The full array of results from the query.
   */
  function pre_render($result) {
    if (!empty($this->row_plugin)) {
      $this->row_plugin->pre_render($result);
    }
  }

  /**
   * Render the display in this style.
   */
  function render() {
    if (empty($this->row_plugin)) {
      vpr('views_plugin_style_default: Missing row plugin');
      return;
    }

    // Group the rows according to the grouping field, if specified.
    $sets = $this->render_grouping($this->view->result, $this->options['grouping']);

    // Render each group separately and concatenate.  Plugins may override this
    // method if they wish some other way of handling grouping.
    $output = '';
    foreach ($sets as $title => $records) {
      $rows = array();
      foreach ($records as $label => $row) {
        $rows[] = $this->row_plugin->render($row);
      }
      $output .= theme($this->theme_functions(), $this->view, $this->options, $rows, $title);
    }
    return $output;
  }

  /**
   * Group records as needed for rendering.
   *
   * @param $records
   *   An array of records from the view to group.
   * @param $grouping_field
   *   The field id on which to group.  If empty, the result set will be given
   *   a single group with an empty string as a label.
   * @return
   *   The grouped record set.
   */
  function render_grouping($records, $grouping_field = '') {
    $sets = array();
    if ($grouping_field) {
      foreach ($records as $row) {
        $grouping = '';
        // Group on the rendered version of the field, not the raw.  That way,
        // we can control any special formatting of the grouping field through
        // the admin or theme layer or anywhere else we'd like.
        if (isset($this->view->field[$grouping_field])) {
          $grouping = $this->view->field[$grouping_field]->theme($row);
        }
        $sets[$grouping][] = $row;
      }
    }
    else {
      // Create a single group with an empty grouping field.
      $sets[''] = $records;
    }
    return $sets;
  }

  function validate() {
    $errors = parent::validate();

    if ($this->uses_row_plugin()) {
      $plugin = $this->display->handler->get_plugin('row');
      if (empty($plugin)) {
        $errors[] = t('Style @style requires a row style but the row plugin is invalid.', array('@style' => $this->definition['title']));
      }
    }
    return $errors;
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
 *
 * @see hook_views_plugins
 */

/**
 * @}
 */

/**
 * @defgroup views_argument_default_plugins Views' argument default plugins
 * @{
 *
 * Allow specialized methods of filling in arguments when they aren't
 * provided.
 *
 * @see hook_views_plugins
 */

/**
 * @}
 */

/**
 * @defgroup views_argument_validate_plugins Views' argument validate plugins
 * @{
 *
 * Allow specialized methods of validating arguments.
 *
 * @see hook_views_plugins
 */

/**
 * @}
 */

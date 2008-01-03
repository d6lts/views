<?php
// $Id$
/**
 * @file
 * Provide views data and handlers for node.module
 */

/**
 * @defgroup views_node_module node.module handlers
 *
 * Includes the tables 'node', 'node_revisions' and 'history'.
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function node_views_data() {
  // Basic table information.

  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['node']['table']['group']  = t('Node');

  // Advertise this table as a possible base table
  $data['node']['table']['base'] = array(
    'field' => 'nid',
    'title' => t('Node'),
  );

  // For other base tables, explain how we join
  $data['node']['table']['join'] = array(
    'users' => array(
      'handler' => 'views_join', // this is actually optional
      'arguments' => array('node', 'users', 'uid', 'uid'),
     ),
    'node_revisions' => array(
      'arguments' => array('node', 'node_revisions', 'nid', 'nid'),
     ),
  );
  // Provide output plugins specifically for this base type.
  $data['node']['table']['plugins'] = array(
    'row' => array(
      'node' => array(
        'title' => t('Node'),
        'help' => t('Display the node with standard node view.'),
        'handler' => 'views_row_plugin_node_view',
      ),
    ),
  );

  // ----------------------------------------------------------------
  // Fields

  // title
  $data['node']['title'] = array(
    'title' => t('Title'), // The item it appears as on the UI,
    'help' => t('The title of the node.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'field' => 'title', // the real field
      'group' => t('Node'), // The group it appears in on the UI,
      'handler' => 'views_handler_field_node',
      'arguments' => array(TRUE), // arguments to the handler
     ),
  );

  // nid
  $data['node']['nid'] = array(
    'title' => t('Nid'),
    'help' => t('The node ID of the node'), // The help that appears on the UI,
    // Information for displaying the nid
    'field' => array(
      'handler' => 'views_handler_field_node',
      'arguments' => array(TRUE),
    ),
    // Information for accepting a nid as an argument
    'argument' => array(
      'handler' => 'views_handler_argument',
      'arguments' => array('title'),
    ),
    // Information for accepting a nid as a filter
    'filter' => array(
      'handler' => 'views_handler_filter',
    ),
    // Information for sorting on a nid.
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // created field
  $data['node']['created'] = array(
    'title' => t('Post date'), // The item it appears as on the UI,
    'help' => t('The date the node was posted.'), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_date',
      'arguments' => array(TRUE),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // changed field
  $data['node']['changed'] = array(
    'title' => t('Updated date'), // The item it appears as on the UI,
    'help' => t('The date the node was last updated.'), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_date',
      'arguments' => array(TRUE),
    ),
  );

  // Node type
  $data['node']['type'] = array(
    'title' => t('Type'), // The item it appears as on the UI,
    'help' => t('The type of a node (for example, "blog entry", "forum post", "story", etc)'), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_node_type',
      'arguments' => array(TRUE),
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_node_type',
    ),
  );

  // published status
  $data['node']['status'] = array(
    'title' => t('Published'), // The item it appears as on the UI,
    'help' => t('The published status of the node.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'arguments' => array(TRUE), // arguments to the handler
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // promote status
  $data['node']['promote'] = array(
    'title' => t('Promoted to front page'), // The item it appears as on the UI,
    'help' => t('The front page of the node.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'arguments' => array(TRUE), // arguments to the handler
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // sticky
  $data['node']['sticky'] = array(
    'title' => t('Sticky'), // The item it appears as on the UI,
    'help' => t('Whether or not the node is sticky.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'arguments' => array(TRUE), // arguments to the handler
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // ----------------------------------------------------------------------
  // Node revisions table

  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['node']['table']['group']  = t('Node');

  // Advertise this table as a possible base table
  $data['node']['table']['base'] = array(
    'field' => 'vid',
    'title' => t('Node revisions'),
  );

  // For other base tables, explain how we join
  $data['node']['table']['join'] = array(
    'node' => array(
      'arguments' => array('node_revisions', 'node', 'vid', 'vid'),
     ),
    'user' => array(
      'arguments' => array('node_revisions', 'node', 'vid', 'vid'),
     ),
  );

  // Body field
  $data['node_revisions']['body'] = array(
    'title' => t('Body'), // The item it appears as on the UI,
    'help' => t('The actual, full data in the body field.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'field' => 'body', // the real field
      'group' => t('Node'), // The group it appears in on the UI,
      'handler' => 'views_handler_field_markup',
      'arguments' => array(FALSE, 'format'), // arguments to the handler
     ),
  );
  return $data;
}

/**
 * Field handler to provide simple renderer that allows linking to a node.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_node extends views_handler_field {
  /**
   * Override init function to provide generic option to link to node.
   */
  function init(&$view, &$data) {
    parent::init($view, $data);
    if (isset($data->options['link_to_node']) && $view->base_table != 'node') {
      $this->additional_fields[] = $nid;
      $this->nid_field = 'node_nid';
    }
    else {
      $this->nid_field = 'nid';
    }
  }

  /**
   * Provide link to node option
   */
  function options_form(&$form) {
    $form['link_to_node'] = array(
      '#title' => t('Link this field to its node'),
      '#type' => 'checkbox',
      '#default_value' => $this->data->options['link_to_node'],
    );
  }

  function render_link($data, $values) {
    if (!empty($this->data->options['link_to_node'])) {
      return l($data, "node/" . $values->{$this->node_field}, array('html' => TRUE));
    }
    else {
      return $data;
    }
  }

  function render($values) {
    return $this->render_link(check_plain($values->{$this->field_alias}), $values);
  }
}

/**
 * Field handler to translate a node type into its readable form.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_node_type extends views_handler_field_node {
  function render($values) {
    $value = node_get_types('name', $values->{$this->field_alias});
    return $this->render_link(check_plain($value), $values);
  }
}

/**
 * Argument handler to accept a node type.
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_node_type extends views_handler_argument {
  // No constructor is necessary.
  function construct() {
    parent::construct('type');
  }

  /**
   * Override the behavior of summary_name(). Get the user friendly version
   * of the node type.
   */
  function summary_name($data) {
    return $this->node_type($data->{$this->name_alias});
  }

  /**
   * Override the behavior of title(). Get the user friendly version of the
   * node type.
   */
  function title() {
    return $this->node_type($this->argument);
  }

  function node_type($type) {
    $output = node_get_types('name', $type);
    if (empty($output)) {
      $output = t('Unknown node type');
    }
    return check_plain($output);
  }
}

/**
 * @}
 */

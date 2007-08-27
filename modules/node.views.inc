<?php
function node_views_data() {
  // Basic table information.
  $data['node']['table'] = array(
    'group' => t('Node'), // Fields will default to this group
    // Advertise this table as a possible base table
    'base' => array(
      'field' => 'nid',
      'title' => t('Node'),
    ),
    // For other base tables, explain how we join
    'join' => array(
      'users' => array(
        'handler' => 'views_join', // this is actually optional
        'arguments' => array('node', 'users', 'uid', 'uid'),
       ),
    ),
    // Provide output plugins specifically for this base type.
    'plugins' => array(
      'node' => array(
        'title' => t('Node'),
        'help' => t('Display the node with standard node view.'),
        'handler' => 'views_record_plugin_node_view',
      ),
    ),
  );

  // Fields

  // title
  $data['node']['title']['field'] = array(
    'field' => 'title', // the real field
    'group' => t('Node'), // The group it appears in on the UI,
    'title' => t('Title'), // The item it appears as on the UI,
    'help' => t('The title of the node'), // The help that appears on the UI,
    'handler' => 'views_handler_field_node',
    'arguments' => array(TRUE),
  );

  // nid 
  $data['node']['nid'] = array(
    'title' => t('Nid'),
    'help' => t('The node ID of the node'), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_node',
      'arguments' => array(TRUE),
    ),
    'argument' => array(
      'handler' => 'views_handler_argument',
      'arguments' => array('title'),
    ),
    'filter' => array(
      'handler' => 'views_handler_filter',
    ),
  );

  // created field
  $data['node']['created']['title'] = t('Post date'); // The item it appears as on the UI,
  $data['node']['created']['help'] = t('The date the node was posted'); // The help that appears on the UI,
  $data['node']['created']['field']['handler'] = 'views_handler_field_date';
  $data['node']['created']['field']['arguments'] = array(TRUE);
  $data['node']['created']['sort']['handler'] = 'views_handler_sort';

  // changed field
  $data['node']['changed']['field'] = array(
    'title' => t('Updated date'), // The item it appears as on the UI,
    'help' => t('The date the node was last updated'), // The help that appears on the UI,
    'handler' => 'views_handler_field_date',
    'arguments' => array(TRUE),
  );
  $data['node']['type']['field'] = array(
    'title' => t('Type'), // The item it appears as on the UI,
    'help' => t('The type of a node (for example, "blog entry", "forum post", "story", etc)'), // The help that appears on the UI,
    'handler' => 'views_handler_field_node_type',
    'arguments' => array(TRUE),
  );


  return $data;
}

/**
 * Field handler to provide simple renderer that allows linking to a node.
 */
class views_handler_field_node extends views_handler_field {
  /**
   * Override seed function to provide generic option to link to node.
   */
  function seed(&$view, &$data) {
    parent::seed($view, $data);
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
 */
class views_handler_field_node_type extends views_handler_field_node {
  function render($values) {
    $value = node_get_types('name', $values->{$this->field_alias});
    return $this->render_link(check_plain($value), $values);
  }
}

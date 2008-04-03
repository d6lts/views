<?php
// $Id$
/**
 * @file
 * Provide views data and handlers that aren't tied to any other module.
 */

/**
 * @defgroup views_views_module miscellaneous handlers
 *
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function views_views_data() {
  $data['views']['table']['group'] = t('Global');
  $data['views']['table']['join'] = array(
    '#global' => array(),
  );
  $data['views']['random'] = array(
    'title' => t('Random'),
    'help' => t('Randomize the display order.'),
    'sort' => array(
      'handler' => 'views_handler_sort_random',
    ),
  );

  return $data;
}

class views_handler_sort_random extends views_handler_sort {
  function query() {
    global $db_type;
    switch ($db_type) {
      case 'mysql':
      case 'mysqli':
        $formula = 'RAND()';
        break;
      case 'pgsql':
        $formula = 'RANDOM()';
        break;
    }
    if (!empty($formula)) {
      $this->query->add_orderby(NULL, $formula, $this->options['order'], '_' . $this->field);
    }
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['order']['#access'] = FALSE;
  }
}
/**
 * @}
 */

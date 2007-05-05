<?php

/**
 * This include file implements views functionality on behalf of profile.module
 */

function profile_views_tables() {
  // add all profile fields as possible display fields and filters
  $profile_fields = profile_views_get_fields();
  foreach ($profile_fields as $field) {
    $tables["$field->name"] = views_new_table('profile_values', 'internal', 'users', 'uid', 'uid', array( 'fid' => $field->fid ) );
    profile_views_add_field($tables["$field->name"], $field);
    profile_views_add_sort($tables["$field->name"], $field);
    profile_views_add_filter($tables["$field->name"], $field);
  }

  return $tables;  
}

function profile_views_arguments() {

}

function profile_views_default_views() {

}

/**
 * Get all profile fields
 */
function profile_views_get_fields() {
        
    $fields = array();
        
    $results = db_query("SELECT * FROM {profile_fields} ORDER BY category, weight");
    
    while ($row = db_fetch_object($results)) { 
        // don't include private fields
        if (user_access('administer users') || $row->visibility != PROFILE_PRIVATE) {
            // make this work for default and modified profile.module
            //$row->options = unserialize($row->options);
            if( !empty($row->options) ){
              if( unserialize($row->options) == false ){
                // unserialized fields default version
                $options = $row->options;
                unset($row->options);
                $row->options['selection'] = $options;
              }
              else {
                // serialized fields or modified version
                $row->options = unserialize($row->options);
              }
            }
            $fields[] = $row;
        }
    }
    return $fields;
}

/**
 * Add profile fields to view table
 */
function profile_views_add_field(&$table, $field) {

  $name = 'value';
  $label = t('Profile: @field-name', array('@field-name' => $field->title));
  
  switch ($field->type) {
    case 'vocabulary':
      $help = t('This will display all options of the profile field %field-name. ', array('%field-name' => $field ->title));
      $others = array(
        'sortable' => true,
        'handler' => 'views_handler_field_profile_vocabulary',
      );
      break;
    case 'selection':
      $help = t('This will display all options of the profile field %field-name. ', array('%field-name' => $field ->title));
      $others = array(
        'sortable' => true,
        'handler' => 'views_handler_field_profile_selection',
      );
      break;
    case 'date':
      $help = t('Display the date of the %field-name field.', array('%field-name' => $field ->title));
      $others = array(
        'sortable' => false,
        'handler' => 'views_handler_field_profile_date',
      );
      break;      
    case 'checkbox':
      $help = t('Checkbox based profile field help');
      $others = array(
        'sortable' => true,
        'handler' => 'views_handler_field_profile_checkbox',
      );
       break;
    case 'textarea':
      $help = t('Other types based profile field help');
      $others = array(
        'sortable' => true,
        'handler' => 'views_handler_field_profile_textarea',
      );
      break;
    default:
      $help = t('Other types based profile field help');
      $others = array(
        'sortable' => true,
        'handler' => 'views_handler_field_profile_default',
      );
  }

  views_table_add_field($table, $name, $label, $help, $others);

}

/**
 * Add profile fields to sort table
 */
function profile_views_add_sort(&$table, $field) {

  $name = 'value';
  $label = t('Profile: @field-name', array('@field-name' => $field->title));
  $others = array();
  
  
  switch ($field->type) {
    case 'vocabulary':
      $help = t('This allows you to sort by vocabulary terms');
      break;
    case 'selection':
      $help = t('This allows you to sort by selection options');
      break;
    case 'date':
      $other = array(
        'handler' => 'views_handler_sort_profile_date',
      );
      $help = t('This allows you to sort by date');
      break;      
    case 'checkbox':
      $help = t('This allows you to sort yes/no');
       break;
    default:
      $help = '';
  }

  views_table_add_sort($table, $name, $label, $help, $others);

}

/**
 * Add profile filters to view table
 */
function profile_views_add_filter(&$table, $field) {

  $name = 'value';
  $label = t('Profile: @field-name', array('@field-name' => $field->title));
  
  switch ($field->type) {
    case 'vocabulary':
      $help = t('Taxonomy based profile field help');
      $others = array(
        'list' => 'views_handler_filter_tid_by_voc',
        'value-type' => 'array',
        'option' => 'string',
        'operator' => 'views_handler_operator_andor',
        'handler' => 'views_handler_filter_profile_andor',
        'vocabulary' =>  $field->options['vocabulary'],
      );
      break;
    case 'selection':
      $help = t('Selection based profile field help');
      $others = array(
        'list' => 'views_handler_filter_profile_selection',
        'operator' => 'views_handler_operator_andor',
        'options' => $field->options['selection'],
        'handler' => 'views_handler_filter_profile_andor',
      );
      break;
    case 'date':
      $help = t('This filter allows nodes to be filtered by their creation date.')
        .' '. views_t_strings('filter date');
      $others = array(
        'operator' => 'views_handler_operator_gtlt',
        //'value' => views_handler_filter_date_value_form(),
        'value' => views_handler_filter_profile_date_value_form(),
        'handler' => 'views_handler_filter_profile_date',
        'value-type' => 'array',
        'fid' =>  $field->fid,
      );
      break; 
    case 'checkbox':
      $help = t('Checkbox based profile field help');
      $others = array(
        'operator' => array('=' => 'Equals'),
        'list' => 'views_handler_operator_yesno',
        //'value-type' => 'array',
        //'list-type' => 'select',
      );
       break;
    default:
      $help = t('Other types based profile field help');
      $others = array(
        'list-type' => 'list',
        'operator' => 'views_handler_operator_like',
        'handler' => 'views_handler_filter_like',
      );
  }

  views_table_add_filter($table, $name, $label, $help, $others);

}

/* FIELD HANDLER METHODS */

/*
 * Format as a field as a email address.
 */
function views_handler_field_email($fieldinfo, $fielddata, $value, $data) {
  return l($value, 'mailto:' . $value);
}

/**
 * Display a profile field of type 'vocabulary'
 */
function views_handler_field_profile_vocabulary($fieldinfo, $fielddata, $value, $data) {
  return _profile_field_vocabulary_getlist( unserialize($value) , 'taxonomy/term/' );
}

/**
 * Display a profile field of type 'selection'
 */
function views_handler_field_profile_selection($fieldinfo, $fielddata, $value, $data) {  
  $value = (unserialize($value) === false) ? $value : unserialize($value);
  if ( !is_array($value)) { $value = array($value); } // handles cases where values were set before this new profile module was put in place
  return check_plain(implode(', ',$value));
}

/**
 * Default display method for a profile field
 */
function views_handler_field_profile_textarea($fieldinfo, $fielddata, $value, $data) {
  $value = (unserialize($value) === false) ? $value : unserialize($value);
  return check_markup($value);
}

/**
 * Default display method for a profile field
 */
function views_handler_field_profile_default($fieldinfo, $fielddata, $value, $data) {
  $value = (unserialize($value) === false) ? $value : unserialize($value);
  return check_plain($value);
}

/**
 * Display a profile field of type 'checkbox', view as tick mark
 */
function views_handler_field_profile_checkbox($fieldinfo, $fielddata, $value, $data) {
  $value = (unserialize($value) === false) ? $value : unserialize($value);
  return (check_plain($value) == '0') ? '&#10007;' : '&#10003;';
}

/**
 * Display a profile field of type 'date'
 */
function views_handler_field_profile_date($fieldinfo, $fielddata, $value, $data) {
  $format = substr(variable_get('date_format_short', 'm/d/Y - H:i'), 0, 5);
  // Note: Avoid PHP's date() because it does not handle dates before
  // 1970 on Windows. This would make the date field useless for e.g.
  // birthdays.
  $value = unserialize($value);
  $replace = array('d' => sprintf('%02d', $value['day']),
                   'j' => $value['day'],
                   'm' => sprintf('%02d', $value['month']),
                   'M' => map_month($value['month']),
                   'Y' => $value['year'],
                   'H:i' => null,
                   'g:ia' => null);
  return strtr($format, $replace);
}

/* SORT HANDLER METHODS */

function views_handler_sort_profile_date($action, &$query, $sortinfo, $sort) {
  /*
  switch ($GLOBALS['db_type']) {
    case 'mysql':
    case 'mysqli':
      $query->add_orderby('', "rand()", "ASC");
      break;
    case 'pgsql':
      $query->add_orderby('', "random()", "ASC");
      break;
  }
  */
}

/* FILTER LIST METHODS */

function views_handler_filter_profile_selection($op, $filter) {
  $options = array();
  foreach ( explode("\r", check_plain($filter['options'])) as $option) {
    $options[trim($option)] = trim($option);
  }
  return $options;
}

/* FILTER HANDLER FUNCTIONS */

/**
 * Handle a profile date fild.
 *
 * The problem here is that profile date fields are stored as serialized data.
 * Doing any other operation that matching the date is difficult. 
 * possibilities include:
 *   1.) hack profile.module to store date always with same number of digits, 
 *       then user mysql STR_TO_DATE() function.
 *   2.) Fetch the date here, unserialize, filter out users that match criteria
 */
function views_handler_filter_profile_date($op, $filter, &$filterinfo, &$query) {
    
  // Approach 2:
  // - read out date, unserialize
  // - compare to input date
  // - filter out matching users: WHERE users.uid IN(match1, match2, match3)
  if( empty($filter['value']) ) return;
  // CAREFUL! this may result in negative timestamp...

  if( is_array($filter['value']) ){
    // custom selection boxes for date entry were used
    $value = mktime(0, 0, 0, $filter['value']['month'], $filter['value']['day'], $filter['value']['year']);
  }
  else {
    // compatibility with old method of entering dates in a textfield
    $value = $filter['value'] == 'now' ? time() : strtotime($filter['value']);  
  }

  $users = array();
  $results = db_query("SELECT uid, value FROM {profile_values} WHERE fid = %d", $filterinfo['fid']);
  while ($profile = db_fetch_object($results)) { 
    $date = unserialize($profile->value);
    // Epoch problem again...
    $time = mktime(0, 0, 0, $date['month'], $date['day'], $date['year']);
    switch($filter['operator']){
      case '>':
        if($time > $value) $users[] = $profile->uid;
        break;
      case '>=':
        if($time >= $value) $users[] = $profile->uid;
        break;
      case '=':
        if($time == $value) $users[] = $profile->uid;
        break;
      case '!=':
        if($time != $value) $users[] = $profile->uid;
        break;
      case '<=':
        if($time <= $value) $users[] = $profile->uid;
        break;
      case '<':
        if($time < $value) $users[] = $profile->uid;
        break;
    }
  }
  
  if( empty($users) ) $users[] = 'NULL';


  // TODO: make this owrk for date entry into textfield
  if( checkdate($filter['value']['month'], $filter['value']['day'], $filter['value']['year']) ){
    $query->ensure_table('users');
    $query->add_where('users.uid IN(' . implode(', ', $users) .')');
  }

}

/**
 * Custom filter for andor operator for serialized profile fields.
 * Note: This sunstring matching method may not be the most elegant method, 
 * yet working, and anyone is free to point out a better approach to me..
 */
function views_handler_filter_profile_andor($op, $filter, $filterinfo, &$query) {

  if( empty($filter['value']) ) return;

  switch ($op) {
    case 'handler':
      $table = $filterinfo['table'];
      $column = $filterinfo['field'];
      if (empty($column)) {
        $fieldbits = explode('.', $filter['field']);
        $column = $fieldbits[1];
      }
      $field = "$table.$column";
      $query->ensure_table($table);

      $where = array();
      $args = array();

      $operator = ($filter['operator'] == 'NOR') ? 'NOT LIKE' : 'LIKE';
      
      // for 'force single' convert to array
      if( !is_array($filter['value']) ) $filter['value'] = array($filter['value']);
      
      foreach($filter['value'] as $value){
        //$where[] = "%s $operator '%%\"%s\"%%'"; // use this line with modified version of profile.module
        $where[] = "%s $operator '%s'"; // use this line with original version of profile.module
        $args[]  = $field;
        $args[]  = $value;
      }
      
      $operator = ($filter['operator'] == 'NOR') ? 'AND' : $filter['operator'];
      $where = implode(" $operator ", $where);
      $query->add_where("$where", $args);
      break;
  }
  
}

/**
 * Drupal builtin date type does not allow to not have a 
 * value, hence build custom date field here
 */
function views_handler_filter_profile_date_value_form() {

  $form['#tree'] = true;
  $form['day'] = array(
    '#prefix' => '<div class="container-inline">',
    '#type' => 'select',
    '#options' => array('0' => '--') + drupal_map_assoc(range(1,31)),
  );
  $form['month'] = array(
    '#type' => 'select',
    '#options' => array('0' => '--') + drupal_map_assoc(range(1,12), 'map_month'),
  );
  $form['year'] = array(
    '#type' => 'select',
    '#options' => array('0' => '--') + drupal_map_assoc(range(2006,2050)),
    '#suffix' => '</div>',
  );
  $form['#after_build'] = array('views_handler_filter_profile_after_build');
  return $form;
}


function views_handler_filter_profile_after_build($form_element) {
  // get rid of 'edit' in name
  $form_element['year']['#name'] = $form_element['#name'] . '[year]';
  $form_element['month']['#name'] = $form_element['#name'] . '[month]';
  $form_element['day']['#name'] = $form_element['#name'] . '[day]';
  // set the select elements to the previously chosen date
  $form_element['year']['#value'] = $form_element['#default_value']['year'];
  $form_element['month']['#value'] = $form_element['#default_value']['month'];
  $form_element['day']['#value'] = $form_element['#default_value']['day'];  
  return $form_element;
}

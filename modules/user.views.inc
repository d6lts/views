<?php
function user_views_data() {
  $data['users']['table']['base'] = array(
    'field' => 'uid',
    'title' => t('User'),
  );

  $data['users']['table']['join']['node']['handler'] = 'views_join';
  $data['users']['table']['join']['node']['arguments'] = array('users', 'node', 'uid', 'uid');
  $data['users']['table']['group'] = t('User');

  $data['users']['name']['field'] = array(
    'group' => t('User'), // The group it appears in on the UI,
    'title' => t('Name'), // The item it appears as on the UI,
    'help' => t('The user/author name.'), // The help that appears on the UI,
    'handler' => 'views_handler_field',
    'arguments' => array(TRUE),
  );

  return $data;
}
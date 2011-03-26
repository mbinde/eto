<?php

function vyew_edit_user_room($uid) {
  $output = "";

  $target_user = user_load($uid);
  if (!$target_user) {
    $target_user = user_load(array('name' => $uid));
    if (!$target_user) {
      return drupal_not_found();
    } else {
      $uid = $target_user->uid;
    }
  }

  $output .= "<p>To get the API key for a user:</p>";
  $output .= "<ol>";
  $output .= "<li>Create an email address for the user that you can access.</li>";
  $output .= "<li>Sign them up for Vyew: " . l('https://vyew.com/go/register', 'https://vyew.com/go/register', array('absolute' => TRUE,
														     'attributes' => array('target' => '_blank')));
  $output .= "<li>Get their API key and secret: " . l('http://vyew.com/api/public/', 'http://vyew.com/api/public/', array('absolute' => TRUE,
															  'attributes' => array('target' => '_blank')));
  $output .= "</ol>";

  $node = vyew_room_settings_load($uid);
  $output .= drupal_get_form('vyew_edit_room_form', $node, array('uid' => $uid));

  return $output;
}

function vyew_edit_room_form(&$form_state, $node, $vars) {
  $form['uid'] = array (
			    '#type' => 'hidden',
			    '#value' => ($node->uid ? $node->uid : $vars['uid']),
			    );
  $form['vyew_room_id'] = array (
			    '#type' => 'hidden',
			    '#value' => $node->vyew_room_id,
			    );
  $form['vyew_username'] = array (
				  '#type' => 'textfield',
				  '#title' => 'Vyew.com username for this user/room',
				  '#description' => 'Optional',
				  '#default_value' => $node->vyew_username,
				  );
  $form['vyew_pass'] = array (
				  '#type' => 'textfield',
				  '#title' => 'Vyew.com password for this user/room',
				  '#description' => 'Optional',
				  '#default_value' => $node->vyew_pass,
				  );
  $api_link = l('http://vyew.com/api/public/', 'http://vyew.com/api/public/', array('absolute' => TRUE,
										    'attributes' => array('target' => '_blank')));
  $form['api_key'] = array (
				  '#type' => 'textfield',
				  '#title' => 'API Key',
				  '#description' => 'If this is not filled in, no room will be created.<br>Get from ' . $api_link,
				  '#default_value' => $node->api_key,
				  );
  $form['api_secret'] = array (
				  '#type' => 'textfield',
				  '#title' => 'API Secret',
				  '#description' => 'If this is not filled in, no room will be created.<br>Get from ' . $api_link,
				  '#default_value' => $node->api_secret,
				  );
  $form['room_title'] = array (
				  '#type' => 'textfield',
				  '#title' => 'Room Title',
				  '#description' => 'Optional',
				  '#default_value' => $node->room_title,
				  );

  $form['submit'] = array (
			   '#type' => 'submit',
			   '#value' => 'Save Settings',
			   );
  $form['#submit'] = array('vyew_edit_room_form_submit');
  return $form;
}

function vyew_edit_room_form_submit($form, &$form_state) {
  // insert if new, otherwise update
  if ($form_state['values']['vyew_room_id'] > 0) {
    vyew_edit_room_form_update($form_state['values']);
  } else {
    vyew_edit_room_form_insert($form_state['values']);
  }

  drupal_set_message("Setting saved successfully.  " . l('Click here', '#') . " to visit the room.", "status");

  $form_state['redirect'] = 'users/' . $form_state['values']['uid'];

}

function vyew_edit_room_form_insert($n) {
  global $user;

  db_query("INSERT INTO {eto_vyew_room}
               (uid, status, vyew_username, vyew_pass, api_key, api_secret, room_title)
            VALUES
               (%d, %d, '%s', '%s', '%s', '%s', '%s')",
	   $n['uid'], $n['status'], $n['vyew_username'], $n['vyew_pass'], $n['api_key'], $n['api_secret'], $n['room_title']);

  watchdog('vyew',
	   "@user_name inserted a new vyew room config for UID @uid",
	   array("@user_name" => $user->name,
		 "@uid" => $uid));
}


function vyew_edit_room_form_update($n) {
  global $user;

  db_query("UPDATE {eto_vyew_room}
               uid = %d, 
               status = %d, 
               vyew_username = '%s', 
               vyew_pass = '%s', 
               api_key = '%s', 
               api_secret = '%s', 
               room_title = '%s'
            WHERE
               vyew_room_id = %d",
	   $n['uid'], $n['status'], $n['vyew_username'], $n['vyew_pass'], $n['api_key'], $n['api_secret'], $n['room_title'],
	   $n['vyew_room_id']);

  watchdog('vyew',
	   "@user_name updated a vyew room config for UID @uid",
	   array("@user_name" => $user->name,
		 "@uid" => $uid));
}
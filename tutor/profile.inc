<?php

  // eventually this is part of the generic, non-application section, since tutors keep their profiles up to date


function tutor_profile_form(&$form_state, $node, $vars) {
  $form['profile_id'] = array (
				   '#type' => 'hidden',
				   '#value' => ($node->profile_id ? $node->profile_id : 0),
				   );
  $form['uid'] = array (
			    '#type' => 'hidden',
			    '#value' => ($node->uid ? $node->uid : $vars['uid']),
			    );

  $form['first_name'] = array (
			       '#type' => 'textfield',
			       '#title' => 'First Name',
			       '#default_value' => $node->first_name,
			       );

  $form['last_name'] = array (
			       '#type' => 'textfield',
			       '#title' => 'Last Name',
			       '#default_value' => $node->last_name,
			       );

  $form['education'] = array(
			     '#type' => 'textarea',
			     '#title' => 'Education',
			     '#default_value' => $node->education
			     );

  $form['general_comments'] = array(
				    '#type' => 'textarea',
				    '#title' => 'Any additional comments',
				    '#default_value' => $node->general_comments,
				    );
  $form['notification_method'] = array (
					'#type' => 'select',
					'#title' => 'Notification method',
					'#options' => array ('0' => '--Select a notification method--',
							     'email' => 'Email',
							     'text' => 'Text message'),
					'#default_value' => ($node->notification_method ? $node->notification_method : 0),
					);
  $form['notification_address'] = array (
					 '#type' => 'textfield',
					 '#title' => 'Notification address',
					 '#description' => 'Either an email or phone number, depending on what you selected for the method above',
					 '#default_value' => $node->notification_address,
					 );

  $form['certified'] = array (
			      '#type' => 'fieldset',
			      '#title' => 'Certified teacher',
			      '#collapsible' => TRUE,
			      '#collapsed' => FALSE
			      );
  $form['certified']['certified_teacher'] = array (
				      '#type' => 'checkbox',
				      '#title' => 'Yes, I am a certified teacher.',
				      '#default_value' => ($node->certified_teacher ? 'checked' : ''),
				      );
  $form['certified']['certified_states'] = array (
						  '#type' => 'select',
						  '#multiple' => TRUE,
						  '#title' => 'States you are certified to teach in',
						  '#options' => _get_states(),
						  '#default_value' => explode(',', $node->certified_states),
						  );


  $form['save'] = array (
			   '#type' => 'submit',
			   '#value' => 'Save this section',
			   );

  $form['#submit'] = array('tutor_profile_form_submit');
  

  return $form;
}

function tutor_profile_form_submit($form, &$form_state) {

  if ($form_state['values']['profile_id'] > 0) {
    tutor_profile_update($form_state['values']);
  } else {
    tutor_profile_insert($form_state['values']);
  }

  drupal_set_message("Your profile has been saved successfully.", "status");

  global $user;
  if ($user->uid == $form_state['values']['uid']) {
    $form_state['redirect'] = 'user/tutor/application';
  } else {
    $form_state['redirect'] = 'users/' . $form_state['values']['uid'] . '/tutor/application';
  }
}


function tutor_profile_load($uid) {
  $result = db_query("SELECT * FROM {eto_tutor_profile} WHERE uid = %d",
		     $uid);
  return db_fetch_object($result);
}


function tutor_profile_validate ($node) {
  // make sure we filled it out properly

  $required_fields = array (
			    'first_name' => 'enter your first name.',
			    'last_name' => 'enter your last name.',
			    'education' => 'enter your education details.',
			    'general_comments' => 'write something about yourself in the comments section.',
			    'notification_method' => 'select a notification method.',
			    'notification_address' => 'specify what email or phone number to use to notify you.',
			    );

  $messages = array();

  foreach ($required_fields as $f => $text) {
    if (empty($node->$f)) {
      $messages[] = 'You need to ' . $text;
    }
  }
  if ($node->certified_teacher && (empty($node->certified_states))) {
    $messages[] = "If you are a certified teacher you must select what states you are certified in.";
  }
  return $messages;
}



function tutor_profile_insert($n) {
  global $user;

  $states = implode(',', $n['certified_states']);
  $now = time();

  db_query("INSERT INTO {eto_tutor_profile}
               (first_name, last_name, education, general_comments, notification_method,
                notification_address, certified_teacher, certified_states, uid,
                created, changed)
            VALUES
               ('%s', '%s', '%s', '%s', '%s', '%s', %d, '%s', %d, %d, %d)",
	   $n['first_name'], $n['last_name'], $n['education'], $n['general_comments'], $n['notification_method'],
           $n['notification_address'], $n['certified_teacher'], $states, $n['uid'],
	   $now, $now);

  watchdog('profiles', 
	   "@user_name inserted a new tutor profile",
	   array("@user_name" => $user->name),
	   WATCHDOG_NOTICE,
	   l('view', 'user/' . $n['uid'] . 'tutor/application/profile')
	   );
}


function tutor_profile_update($n) {
  global $user;

  $states = implode(',', $n['certified_states']);

  db_query("UPDATE {eto_tutor_profile} SET
               first_name = '%s',
               last_name = '%s',
               education = '%s',
               general_comments = '%s',
               notification_method = '%s',
               notification_address = '%s',
               certified_teacher = '%s',
               certified_states = '%s',
               uid = %d,
               changed = %d
            WHERE
               profile_id = %s",
	   $n['first_name'], $n['last_name'], $n['education'], $n['general_comments'], $n['notification_method'],
           $n['notification_address'], $n['certified_teacher'], $states, $n['uid'],
	   time(),
	   $n['profile_id']
	   );

  watchdog('profiles', 
	   "@user_name updated a profile",
	   array("@user_name" => $user->name)
	   );

}


<?php

function tutor_profile_form(&$form_state, $node, $vars) {
  $form['profile_id'] = array (
				   '#type' => 'hidden',
				   '#value' => ($node->profile_id ? $node->profile_id : 0),
				   );
  $form['uid'] = array (
			    '#type' => 'hidden',
			    '#value' => ($node->uid ? $node->uid : $vars['uid']),
			    );

  $form['where_heard'] = array (
				'#type' => 'textfield',
				'#title' => 'Where did you get the link to this online application',
				'#required' => TRUE,
				'#default_value' => $node->where_heard,
				);
  $form['referral'] = array (
				'#type' => 'textfield',
				'#title' => 'If you have a referral code, enter it here',
				'#default_value' => $node->referral,
				);

  $form['first_name'] = array (
			       '#type' => 'textfield',
			       '#title' => 'First Name',
			       '#required' => TRUE,
			       '#default_value' => $node->first_name,
			       );

  $form['last_name'] = array (
			       '#type' => 'textfield',
			       '#title' => 'Last Name',
			       '#default_value' => $node->last_name,
			       '#required' => TRUE,
			       );

  $form['skype_name'] = array (
			       '#type' => 'textfield',
			       '#title' => 'Skype User Name',
			       '#required' => TRUE,
			       '#default_value' => $node->skype_name,
			       );
  $form['current_occupation'] = array (
				       '#type' => 'textfield',
				       '#title' => 'Current Occupation',
				       '#default_value' => $node->current_occupation,
				       );
  $form['education'] = array(
			     '#type' => 'textarea',
			     '#title' => 'College Attended/Degrees Earned/Major',
			     '#required' => TRUE,
			     '#default_value' => $node->education
			     );
  $form['best_subject'] = array (
				 '#type' => 'textfield',
				 '#required' => TRUE,
				 '#title' => 'Please name the subject in which you tutor most often',
				 '#default_value' => $node->best_subject,
				 );
  $form['other_subjects'] = array(
				  '#type' => 'textarea',
				  '#title' => 'Please list other subjects in which you feel comfortable tutoring',
				  '#default_value' => $node->other_subjects,
				  );
  /*
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
  */

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
  $form['crimes'] = array (
			   '#type' => 'textarea',
			   '#title' => 'Please list any misdemeanors, felonies, or crimes for which you have been convicted',
			   '#description' => 'You may omit (1) traffic fines for which you paid a fine of $50 or less (2) any offense committed before your 18th birthday which was finally adjudicated in a juvenile court or under a youth offense law (3) any conviction the record of which has been expunged under Federal or State law, and (4) any conviction set aside under the federal Youth Correction Act or similar State authority',
			   '#required' => TRUE,
			   '#default_value' => $node->crimes
			   );

  $form['background_cost'] = array (
				    '#type' => 'radios',
				    '#options' => yesno(),
				    '#required' => TRUE,
				    '#title' => 'Are you willing to pay $20 for a background check',
				    '#default_value' => $node->background_cost
				    );

  $form['online_tutoring_experience'] = array (
					       '#type' => 'textarea',
					       '#required' => TRUE,
					       '#title' => 'Have you ever tutored online? If yes, please describe your experience',
					       '#default_value' => $node->online_tutoring_experience,
					       );

  $form['tutoring_years'] = array (
				   '#type' => 'textfield',
				   '#required' => TRUE,
				   '#title' => 'How many years of tutoring experience do you have',
				   '#default_value' => $node->tutoring_years,
				   );

  $form['tutoring_students'] = array (
				      '#type' => 'textfield',
				      '#required' => TRUE,
				      '#title' => 'How many students have you tutored',
				      '#default_value' => $node->tutoring_students,
				      );
  $form['tutoring_clients'] = array (
				     '#type' => 'textfield',
				     '#required' => TRUE,
				     '#title' => 'How many private tutoring clients do you currently have',
				     '#default_value' => $node->tutoring_clients,
				     );
  $form['tutoring_rate'] = array (
				     '#type' => 'textfield',
				     '#required' => TRUE,
				     '#title' => 'What is your customary hourly rate for tutoring',
				     '#default_value' => $node->tutoring_rate,
				     );
  
  $form['general_comments'] = array(
				    '#type' => 'textarea',
				    '#title' => 'Please provide additional information about your specific tutoring experience and background',
				    '#default_value' => $node->general_comments,
				    );

  $form['tutoring_level'] = array(
				  '#type' => 'checkboxes',
				  '#title' => 'Which level of students do you feel comfortable tutoring',
				  '#options' => array('elementary' => 'Elementary School',
						      'middle' => 'Middle School',
						      'high' => 'High School',
						      'college' => 'College',
						      'graduate' => 'Graduate'),
				  '#default_value' => ($node->tutoring_level ? $node->tutoring_level : array()),
				  );
  $form['tutoring_focus'] = array (
				   '#type' => 'checkboxes',
				   '#title' => 'Do you focus on any of these special groups? (check all that apply)',
				   '#options' => array('gifted' => 'Gifted/Talented',
						       'homeschool' => 'Homeschool',
						       'remedial' => 'Remedial',
						       'ld' => 'Learning Disabilities',
						       ),
				   '#default_value' => ($node->tutoring_focus ? $node->tutoring_focus : array()),
				   );
  $form['tutoring_hours'] = array (
				   '#type' => 'checkboxes',
				   '#title' => 'What hours are you generally available to tutor',
				   '#options' => array ('day' => 'Homeschool hours (M-F during the day)',
							'after' => 'After School (M-F evenings)',
							'weekend' => 'Weekends',
							),
				   '#default_value' => ($node->tutoring_hours ? $node->tutoring_hours : array()),
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
    $u = eto_user_load($form_state['values']['uid']);
    $form_state['redirect'] = 'users/' . $u->name . '/tutor/application';
  }
}


function tutor_profile_load($uid) {
  if (! is_numeric($uid)) {
    if (is_object($uid)) {
      $uid = $uid->uid;
    } else {
      $u = eto_user_load($uid);
      $uid = $u->uid;
    }
  }
  $result = db_query("SELECT * FROM {eto_tutor_profile} WHERE uid = %d",
		     $uid);
  $obj = db_fetch_object($result);
  foreach ( array('tutoring_level', 'tutoring_focus', 'tutoring_hours') as $key ) {
    $obj->{$key} = unserialize($obj->{$key});
  }
  return $obj;
}


function tutor_profile_validate ($node) {
  // make sure we filled it out properly

  $required_fields = array (
			    'first_name' => 'enter your first name.',
			    'last_name' => 'enter your last name.',
			    'education' => 'enter your education details.',
			    );

  //			    'notification_method' => 'select a notification method.',
  //			    'notification_address' => 'specify what email or phone number to use to notify you.',
  //			    'general_comments' => 'write something about yourself in the comments section.',

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
                notification_address, certified_teacher, certified_states, skype_name,
                current_occupation, best_subject, other_subjects, crimes, background_cost,
                online_tutoring_experience, tutoring_years, tutoring_students, tutoring_clients,
                tutoring_rate, tutoring_level, tutoring_focus, tutoring_hours, where_heard, 
                referral,
                uid, created, changed)
            VALUES
               ('%s', '%s', '%s', '%s', '%s', '%s', %d, '%s', '%s', '%s', '%s', '%s', '%s', %d, '%s',
                '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s',
                %d, %d, %d)",
	   $n['first_name'], $n['last_name'], $n['education'], $n['general_comments'], $n['notification_method'],
           $n['notification_address'], $n['certified_teacher'], $states, $n['skype_name'],
	   $n['current_occupation'], $n['best_subject'], $n['other_subjects'], $n['crimes'],
	   $n['background_cost'], $n['online_tutoring_experience'], $n['tutoring_years'],
	   $n['tutoring_students'], $n['tutoring_clients'], $n['tutoring_rate'],
	   serialize($n['tutoring_level']), serialize($n['tutoring_focus']), serialize($n['tutoring_hours']),
	   $n['where_heard'], $n['referral'],
	   $n['uid'], $now, $now);

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
               skype_name = '%s',
               current_occupation = '%s',
               best_subject = '%s',
               other_subjects = '%s',
               crimes = '%s',
               background_cost = %d,
               online_tutoring_experience = '%s',
               tutoring_years = '%s',
               tutoring_students = '%s',
               tutoring_clients = '%s',
               tutoring_rate = '%s',
               tutoring_level = '%s',
               tutoring_focus = '%s',
               tutoring_hours = '%s',
               where_heard = '%s',
               referral = '%s',
               uid = %d,
               changed = %d
            WHERE
               profile_id = %s",
	   $n['first_name'], $n['last_name'], $n['education'], $n['general_comments'], $n['notification_method'],
           $n['notification_address'], $n['certified_teacher'], $states, $n['skype_name'], $n['current_occupation'],
	   $n['best_subject'], $n['other_subjects'], $n['crimes'], $n['background_cost'], $n['online_tutoring_experience'],
	   $n['tutoring_years'], $n['tutoring_students'], $n['tutoring_clients'], $n['tutoring_rate'], 
	   serialize($n['tutoring_level']), serialize($n['tutoring_focus']), serialize($n['tutoring_hours']),
	   $n['where_heard'], $n['referral'],
	   $n['uid'],
	   time(),
	   $n['profile_id']
	   );

  watchdog('profiles', 
	   "@user_name updated a profile",
	   array("@user_name" => $user->name)
	   );

}

/***********
 *
 * Computer section
 *
 ***********/

function yesno() {
  return array('1' => 'Yes', '0' => 'No');
}

function tutor_computer_form(&$form_state, $node, $vars) {
  $form['computer_id'] = array (
				   '#type' => 'hidden',
				   '#value' => ($node->computer_id ? $node->computer_id : 0),
				   );
  $form['uid'] = array (
			    '#type' => 'hidden',
			    '#value' => ($node->uid ? $node->uid : $vars['uid']),
			    );

  $form['webcam'] = array (
			   '#type' => 'radios',
			   '#options' => yesno(),
			   '#title' => 'Does the computer from which you plan to tutor have a currently have a webcam, either built-in or attached?',
			   '#required' => TRUE,
			   '#default_value' => $node->webcam,
			   );

  $form['connection_type'] = array (
			   '#type' => 'radios',
			   '#options' => yesno(),
			   '#title' => 'Is the computer from which you plan to tutor connected to the internet via ethernet or wireless?',
			   '#required' => TRUE,
			   '#default_value' => $node->connection_type,
			   );
  $form['high_speed'] = array (
			   '#type' => 'radios',
			   '#options' => yesno(),
			   '#title' => 'Does the computer from which you plan to tutor have high-speed internet access?',
			   '#required' => TRUE,
			   '#default_value' => $node->high_speed,
			   );
  $form['skype_knowledge'] = array (
			   '#type' => 'radios',
			   '#options' => yesno(),
			   '#title' => 'Do you know how to make a Skype video call?',
			   '#required' => TRUE,
			   '#default_value' => $node->skype_knowledge,
			   );

  $form['save'] = array (
			   '#type' => 'submit',
			   '#value' => 'Save this section',
			   );

  $form['#submit'] = array('tutor_computer_form_submit');
  

  return $form;
}

function tutor_computer_validate($node) {
  if ($node) {
    return 0;
  } else {
    return 1;
  }
}

function tutor_computer_form_submit($form, &$form_state) {

  if ($form_state['values']['computer_id'] > 0) {
    tutor_computer_update($form_state['values']);
  } else {
    tutor_computer_insert($form_state['values']);
  }

  drupal_set_message("Your computer has been saved successfully.", "status");

  global $user;
  if ($user->uid == $form_state['values']['uid']) {
    $form_state['redirect'] = 'user/tutor/application';
  } else {
    $u = eto_user_load($form_state['values']['uid']);
    $form_state['redirect'] = 'users/' . $u->name . '/tutor/application';
  }
}



function tutor_computer_insert($n) {
  global $user;

  $now = time();

  db_query("INSERT INTO {eto_tutor_computer}
               (webcam, connection_type, high_speed, skype_knowledge,
                uid, created, changed)
            VALUES
               (%d, %d, %d, %d, %d, %d, %d)",
	   $n['webcam'], $n['connection_type'], $n['high_speed'], $n['skype_knowledge'],
           $n['uid'], $now, $now);

  watchdog('profiles', 
	   "@user_name inserted a new tutor computer",
	   array("@user_name" => $user->name),
	   WATCHDOG_NOTICE,
	   l('view', 'user/' . $n['uid'] . 'tutor/application/computer')
	   );
}


function tutor_computer_update($n) {
  global $user;

  db_query("UPDATE {eto_tutor_computer} SET
               webcam = '%s',
               connection_type = '%s',
               high_speed = '%s',
               skype_knowledge = '%s',
               uid = %d,
               changed = %d
            WHERE
               computer_id = %s",
	   $n['webcam'], $n['connection_type'], $n['high_speed'], $n['skype_knowledge'],
           $n['uid'], time(),
	   $n['computer_id']
	   );

  watchdog('profiles', 
	   "@user_name updated a computer",
	   array("@user_name" => $user->name)
	   );

}


function tutor_computer_load($uid) {
  if (! is_numeric($uid)) {
    if (is_object($uid)) {
      $uid = $uid->uid;
    } else {
      $u = eto_user_load($uid);
      $uid = $u->uid;
    }
  }
  $result = db_query("SELECT * FROM {eto_tutor_computer} WHERE uid = %d",
		     $uid);
  $obj = db_fetch_object($result);
  return $obj;
}
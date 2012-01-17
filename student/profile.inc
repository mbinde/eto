<?php

function student_profile_form(&$form_state, $node, $vars) {
  $form['student_id'] = array (
				   '#type' => 'hidden',
				   '#value' => ($node->student_id ? $node->student_id : 0),
				   );
  $form['uid'] = array (
			    '#type' => 'hidden',
			    '#value' => ($node->uid ? $node->uid : $vars['uid']),
			    );

  $form['parent_info'] = array (
				'#type' => 'fieldset',
				'#title' => 'Parent or Guardian Contact Information',
				'#collapsible' => TRUE,
				'#collapsed' => FALSE,
				);
  $form['parent_info']['first_name_parent'] = array (
			       '#type' => 'textfield',
			       '#title' => 'First Name',
			       '#default_value' => $node->first_name_parent,
			       );

  $form['parent_info']['last_name_parent'] = array (
			       '#type' => 'textfield',
			       '#title' => 'Last Name',
			       '#default_value' => $node->last_name_parent,
			       );
  $form['parent_info']['email_parent'] = array (
			       '#type' => 'textfield',
			       '#title' => 'Email',
			       '#default_value' => $node->email_parent,
			       );


  $form['student_info'] = array (
				'#type' => 'fieldset',
				'#title' => 'Student Information',
				'#collapsible' => TRUE,
				'#collapsed' => FALSE,
				);

  $form['student_info']['first_name'] = array (
			       '#type' => 'textfield',
			       '#title' => 'First Name',
			       '#default_value' => $node->first_name,
			       );

  $form['student_info']['last_name'] = array (
			       '#type' => 'textfield',
			       '#title' => 'Last Name',
			       '#default_value' => $node->last_name,
			       );

  $form['student_info']['new_birthdate'] = array(
			     '#type' => 'date_popup',
			     '#date_format' => 'm/d/Y',
			     '#title' => 'Birthdate',
			     '#size' => '10',
			     '#default_value' => ($node->new_birthdate ? $node->new_birthdate : $node->birthdate),
			     );
  $form['student_info']['school'] = array (
					   '#type' => 'textfield',
					   '#title' => 'School student attends',
					   '#default_value' => $node->school,
					   );
  $school_type = school_type_select();
  $form['student_info']['school_type'] = array (
						'#type' => 'radios',
						'#title' => 'School Type',
						'#default_value' => $node->school_type,
						'#options' => $school_type,
						);
  $instruction = instruction_select();
  $form['student_info']['instruction'] = array (
						'#type' => 'radios',
						'#title' => 'Primary Means of Instruction',
						'#default_value' => $node->instruction,
						'#options' => $instruction,
						);
						
  $form['student_info']['grade'] = array(
			     '#type' => 'textfield',
			     '#title' => 'Grade',
			     '#size' => '15',
			     '#default_value' => $node->grade
			     );
  $form['student_info']['textbooks'] = array(
			     '#type' => 'textarea',
			     '#title' => 'Information for Tutors',
			     '#default_value' => $node->textbooks
			     );

  /*
  $form['student_info']['notification_method'] = array (
					'#type' => 'select',
					'#title' => 'Notification method',
					'#options' => array ('0' => '--Select a notification method--',
							     'email' => 'Email',
							     'text' => 'Text message'),
					'#default_value' => ($node->notification_method ? $node->notification_method : 0),
					);
  $form['student_info']['notification_address'] = array (
					 '#type' => 'textfield',
					 '#title' => 'Notification address',
					 '#description' => 'Either an email or phone number, depending on what you selected for the method above',
					 '#default_value' => $node->notification_address,
					 );
*/
  $form['save'] = array (
			   '#type' => 'submit',
			   '#value' => 'Save this section',
			   );

  $form['#submit'] = array('student_profile_form_submit');
  

  return $form;
}

function student_profile_form_submit($form, &$form_state) {

  if ($form_state['values']['student_id'] > 0) {
    student_profile_update($form_state['values']);
  } else {
    student_profile_insert($form_state['values']);
  }

  drupal_set_message("Your profile has been saved successfully.", "status");

  global $user;
  if ($user->uid == $form_state['values']['uid']) {
    $form_state['redirect'] = 'user/student/application';
  } else {
    $u = eto_user_load($form_state['values']['uid']);
    $form_state['redirect'] = 'users/' . $u->name . '/student/application';
  }
}



function student_profile_validate ($node) {
  // make sure we filled it out properly

  $required_fields = array (
			    'first_name' => 'enter your first name.',
			    'last_name' => 'enter your last name.',
			    'birthdate' => 'enter your birthdate.',
			    );
  //			    'notification_method' => 'select a notification method.',
  //			    'notification_address' => 'specify what email or phone number to use to notify you.',

  $messages = array();

  foreach ($required_fields as $f => $text) {
    if (empty($node->$f)) {
      $messages[] = 'You need to ' . $text;
    }
  }
  if ($node->birthdate) {
    $date = date_make_date($node->birthdate, 'UTC');
    $birthdate_time = date_format($date, 'U');
    if ((time() - $birthdate_time) <= 409968000) {
      // require coppa
      $required_fields = array (
				'first_name_parent' => 'enter your parent or guardian\'s first name.',
				'last_name_parent' => 'enter your parent or guardian\'s last name.',
				'email_parent' => 'enter your parent or guardian\'s email address.',
				);
      foreach ($required_fields as $f => $text) {
	if (empty($node->$f)) {
	  $messages[] = 'You need to ' . $text;
	}
      }
    }
  }
  return $messages;
}



function student_profile_insert($n) {
  global $user;

  $now = time();

  if ($n['new_birthdate']) {
    $n['birthdate'] = strtotime(eto_format_date($n['new_birthdate']));
  }

  db_query("INSERT INTO {eto_student_profile}
               (first_name, last_name, first_name_parent, last_name_parent, email_parent,
                birthdate, grade, textbooks, 
                notification_method, notification_address, uid,
                school, school_type, instruction,
                created, changed)
            VALUES
               ('%s', '%s', '%s', '%s', '%s', %d, '%s', '%s', '%s', '%s', %d, '%s', %d, %d,
                %d, %d)",
	   $n['first_name'], $n['last_name'], $n['first_name_parent'], $n['last_name_parent'], $n['email_parent'],
	   $n['birthdate'], $n['grade'], $n['textbooks'],
	   $n['notification_method'], $n['notification_address'], $n['uid'],
	   $n['school'], $n['school_type'], $n['instruction'],
	   $now, $now);

  watchdog('profiles', 
	   "@user_name inserted a new student profile",
	   array("@user_name" => $user->name)
	   );
}


function student_profile_update($n) {
  global $user;

  if ($n['new_birthdate']) {
    $d = date_make_date($n['new_birthdate'], "UTC");
    $n['birthdate'] = date_format($d, 'Y-m-d');
  }

  db_query("UPDATE {eto_student_profile} SET
               first_name = '%s',
               last_name = '%s',
               first_name_parent = '%s',
               last_name_parent = '%s',
               email_parent = '%s',
               birthdate = '%s',
               grade = '%s',
               textbooks = '%s',
               notification_method = '%s',
               notification_address = '%s',
               uid = %d,
               school = '%s',
               school_type = %d,
               instruction = %d,
               changed = %d
            WHERE
               student_id = %s",
	   $n['first_name'], $n['last_name'], $n['first_name_parent'], $n['last_name_parent'], $n['email_parent'],
	   $n['birthdate'], $n['grade'], $n['textbooks'],
	   $n['notification_method'], $n['notification_address'], $n['uid'],
	   $n['school'], $n['school_type'], $n['instruction'],
	   time(),
	   $n['student_id']
	   );

  watchdog('profiles', 
	   "@user_name updated a profile",
	   array("@user_name" => $user->name)
	   );

}


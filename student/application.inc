<?php

function _student_validate_permissions($uid) {
  if (! is_numeric($uid)) {
    if (is_object($uid)) {
      $uid = $uid->uid;
    } else {
      $u = eto_user_load($uid);
      $uid = $u->uid;
    }
  }
  global $user;

  if ($uid) {
    // only allowable if this is us or we have proper permissions
    if ( ! ( ($user->uid == $uid) || user_access("administer students") ) ) {
      return 0;
    } else {
      return 1;
    }
  }
}

function _student_load_application_part($uid, $part) {
  if (! is_numeric($uid)) {
    if (is_object($uid)) {
      $uid = $uid->uid;
    } else {
      $u = eto_user_load($uid);
      $uid = $u->uid;
    }
  }

  if ($part == "consent") {
    return student_application_consent_load($uid);
  } else if ($part == "profile") {
    module_load_include("inc", "student", "profile");
    return student_profile_load($uid);
  } else if ($part == "contact") {
    module_load_include("inc", "eto_admin", "contact");
    return eto_contact_load($uid, 'CONTACT');
  } else {
    return 0;
  }
}

function _student_validate_application_part($node, $part) {
  if ($part == "consent") {
    return student_consent_validate($node);
  } else if ($part == "contact") {    
    return 0; // if it was saved, it's validated
  } else if ($part == "profile") {
    module_load_include("inc", "student", "profile");
    return student_profile_validate($node);
  } else {
    return 1;
  }
}

/***************************************************************************
 *
 * application page
 *
 ***************************************************************************/

function student_application($uid = "") {
  $output = "";
  global $user;

  if (!$uid) {
    $mine = 1;
    $target_user = $user;
    $uid = $user->uid;
  } else {
    $target_user = eto_user_load($uid);
    if (!$target_user) {
      $target_user = eto_user_load(array('name' => $uid));
      if (!$target_user) {
	return drupal_not_found();
      } else {
	// set our UID to be numeric
	$uid = $target_user->uid;
      }
    }
  }
  $uid = $target_user->uid;

  if (! _student_validate_permissions($uid)) {
    return "Permission denied.";
  }
  // else

  $app = _student_load_application_part($uid, 'consent');

  if ($app && ($app->status > 0)) {

    if ($app->status == 1) {
      // pending approval
      if ($mine) {
	$output .= "<p>Thank you for submitting your application.  We will get back to you as soon as possible.</p>";
      } else {
	$output .= "<p>This application has been submitted and is waiting for your approval.</p>";
	$parts_status = _student_display_application_parts ($uid, $mine);
	$output .= $parts_status['output'];
	$output .= "<p>Once you have reviewed the application and made any changes, click below to promote this user to be a student.</p>";
	$node = student_application_consent_load($uid);
	$output .= drupal_get_form('student_application_approval_form', $node);
      }
    } else {
      if ($mine) {
	$output .= "You are already a student!";
      } else {
	$output .= "<p><strong>This user is already a student.</strong>  Their application is included below if you wish to review any pieces of it.  Please note that the contact information and biography may have been changed since when they were submitted.</p>";
	if (! in_array('Student', array_values($target_user->roles))) {
	  drupal_set_message("<strong>Warning:</strong> This user does not have the 'Student' role set, so they are not fully promoted as a student. " . l('Click here to add the student role.', 'user/' . $uid . '/edit'), "warning");
	}
	$parts_status = _student_display_application_parts ($uid, $mine);
	$output .= $parts_status['output'];
      }
    }

  } else {
    if ($mine) { 
      $output .= "<p>When you have completed all parts of the application process, you may submit your application.</p>";
    } else {
      $output .= "<p>Application status:</p>";
    }

    $parts_status = _student_display_application_parts ($uid, $mine);
    $output .= $parts_status['output'];
    if ($parts_status['exists'] == $parts_status['total']) {
      // validate our nodes to make sure the important stuff is handled // FIXME
      if ($parts_status['exists'] == $parts_status['completed']) {
	if ($mine) {
	  $output .= "Your application is complete!  Please review it and make any final changes.  After you submit it, you will not be able to change it again.";
	} else {
	  $output .= $target_user->name . "'s application is complete.  If you wish to submit it for them, please click the submit button below.";
	}
	$node = student_application_consent_load($uid);
	$output .= drupal_get_form('student_application_submit_form', $node);
      } else {
	if ($mine) {
	  $output .= "Please edit and finish the incomplete sections of your application.";
	} else {
	  $output .= "The application is still incomplete and cannot be submitted or approved yet.";
	}
      }
    }
  }
  return $output;
}


function _student_display_application_parts ($uid, $mine = 0) {
  $target_user = eto_user_load($uid);
  $uid = $target_user->uid;

  $status = array();

  $target_user = eto_user_load($uid);
  
  $parts = array('contact' => 'Contact Information',
		 'profile' => 'Profile',
		 'consent' => 'Terms and Conditions',
		 );

  $status['output'] .= "\n\n<table>\n";

  
  foreach ($parts as $key => $text) {
    
    $status['total'] ++;
    
    $data = _student_load_application_part($uid, $key);
    if ($data) {
      $status['exists'] ++;
      $messages = _student_validate_application_part($data, $key);
      if (! $messages) {
	$status['completed'] ++;
	$image = "<img src='/images/green-check.png' alt='Section completed' title='Section completed' width=24 height=24>";
      } else {
	$hover_text = implode("  ", $messages);
	$image = "<img src='/images/in-progress.png' alt=\"$hover_text\" title=\"$hover_text\" width=24 height=24>";
      }
    } else {
      $image = "<img src='/images/question-mark.png' alt='Section not yet started' title='Section not yet started' width=24 height=24>";
    }
    
    if ($mine) {
      $link = "user/student/application/$key";
    } else {
      $link = "users/" . $target_user->name . "/student/application/$key";
    }
    $status['output'] .= "<tr>\n";
    $status['output'] .= "<td>" . l('[edit]', $link) . "</td>";
    $status['output'] .= "<td>$image</td>";
    $status['output'] .= "<td>$text</td>";
    $status['output'] .= "\n</tr>\n";
  }
  $status['output'] .= "</table>";


  return $status;
}


/***************************************************************************
 *
 * final submission subsection
 *
 ***************************************************************************/

function student_application_submit_form(&$form_state, $node) {
  $form['uid'] = array (
			    '#type' => 'hidden',
			    '#value' => $node->uid,
			    );
  $form['submit'] = array (
			   '#type' => 'submit',
			   '#value' => 'Submit application',
			   );
  $form['#submit'] = array('student_application_submit_form_submit');
  return $form;
}

function student_application_submit_form_submit($form, &$form_state) {
  $node = student_application_consent_load($form_state['values']['uid']);
  $node->status = 1;
  student_application_consent_update(to_array($node));
  drupal_set_message("Your application has been submitted successfully!  We will get back to you shortly.", "status");

  global $user;
  if ($user->uid == $form_state['values']['uid']) {
    $form_state['redirect'] = 'user/student/application';
  } else {
    $u = eto_user_load($form_state['values']['uid']);
    $form_state['redirect'] = 'users/' . $u->name . '/student/application';
  }
}

/***************************************************************************
 *
 * student approval subsection
 *
 ***************************************************************************/

function student_application_approval_form(&$form_state, $node) {
  $form['uid'] = array (
			    '#type' => 'hidden',
			    '#value' => $node->uid,
			    );
  $form['submit'] = array (
			   '#type' => 'submit',
			   '#value' => 'Approve this Student',
			   );
  $form['#submit'] = array('student_application_approval_form_submit');
  return $form;
}

function student_application_approval_form_submit($form, &$form_state) {
  $node = student_application_consent_load($form_state['values']['uid']);
  $node->status = 2;

  student_application_consent_update(to_array($node));

  drupal_set_message("Student's application is approved and the student has been notified.", "status");
  drupal_set_message("To finish adding them as a student you *must* select the 'Student' role below and save their account.", "error");
  $u = eto_user_load($form_state['values']['uid']);
  $form_state['redirect'] = 'users/' . $u->name . '/student/application';
}


/***************************************************************************
 *
 * contact subsection
 *
 ***************************************************************************/

function student_application_contact ($name = "") {
  $output = "";
  global $user;

  if (!$name) {
    $name = $user->name;
  }
  $u = eto_user_load(array('name' => $name));
  if (! _student_validate_permissions($u->uid)) {
    return "Permission denied.";
  } 
  // else

  module_load_include('inc', 'eto_admin', 'contact');
  $node = eto_contact_load($u->uid, 'CONTACT');

  $output .= variable_get('eto_contact_form_help_text', '<h3>Please fill out the contact information below:</h3>');

  global $user;
  if ($user->uid == $u->uid) {
    $redirect = 'user/student/application';
  } else {
    $redirect = 'users/' . $u->name . '/student/application';
  }

  $output .= drupal_get_form('eto_contact_form', $node, array('uid' => $u->uid, 
							      'save' => 'Save this section', 
							      'contact_type' => 'CONTACT',
							      'redirect' => $redirect,
							      ));

  return $output;
}



/***************************************************************************
 *
 * profile subsection
 *
 ***************************************************************************/

function student_application_profile ($name = "") {
  $output = "";
  global $user;

  if (!$name) {
    $name = $user->name;
  }
  $u = eto_user_load(array('name' => $name));
  if (! _student_validate_permissions($u->uid)) {
    return "Permission denied.";
  } 
  // else

  module_load_include('inc', 'student', 'profile');
  $node = student_profile_load($u->uid);


  $output .= variable_get('eto_student_profile_form_help_text', '<h3>Please fill out the information below:</h3>');

  if ($node) {
    $messages = _student_validate_application_part($node, 'profile');
    if ($messages) {
      $output .= "<div id='validate-messages'>";
      $output .= implode('<br>', $messages);
      $output .= "</div>";
    }
  }

  $output .= drupal_get_form('student_profile_form', $node, array('uid' => $u->uid));

  return $output;
}



/***************************************************************************
 *
 * consent subsection
 *
 ***************************************************************************/

function student_consent_validate($node) {
  if ($node->terms_and_conditions_agreement == 1) {
    return 0;
  } else {
    return array("You need to accept the terms and conditions.");
  }
}

function student_application_consent($name = "") {
  $output = "";
  global $user;

  if (!$name) {
    $name = $user->name;
  }
  $u = eto_user_load(array('name' => $name));
  if (! _student_validate_permissions($u->uid)) {
    return "Permission denied.";
  }
  // else
  
  $node = student_application_consent_load($u->uid);

  $output .= variable_get('eto_student_consent_form_help_text', '<h3>Please fill out the information below:</h3>');

  if ($node) {
    $messages = _student_validate_application_part($node, 'consent');
    if ($messages) {
      $output .= "<div id='validate-messages'>";
      $output .= implode('<br>', $messages);
      $output .= "</div>";
    }
  }

  $output .= drupal_get_form('student_application_consent_form', $node, array('uid' => $u->uid));

  return $output;
}

/**
 * Implementation of hook_form
 */

function student_application_consent_form(&$form_state, $node, $vars) {

  $form['application_id'] = array (
				   '#type' => 'hidden',
				   '#value' => ($node->application_id ? $node->application_id : 0),
				   );
  $form['uid'] = array (
			    '#type' => 'hidden',
			    '#value' => ($node->uid ? $node->uid : $vars['uid']),
			    );

  $form['terms_and_conditions_agreement'] = array (
				       '#type' => 'textfield',
				       '#title' => 'You must accept the Terms of Use to use our service.  Please indicate your acceptance by typing I AGREE (all caps) in the box',
				       '#default_value' => ($node->terms_and_conditions_agreement ? "I AGREE" : ""),
				       '#required' => TRUE,
				       );

  $form['tnc_text'] = array ('#value' => variable_get('terms-and-conditions', ''));

  // don't ever let them uncheck it!

  if ($node->terms_and_conditions_agreement == 1) {
    $form['terms_and_conditions_agreement']['#disabled'] = TRUE;
  } else {
    $form['save'] = array (
			   '#type' => 'submit',
			   '#value' => 'Save this section',
			   );
  }

  $form['#validate'][] = 'student_application_consent_validate';
  $form['#submit'] = array('student_application_consent_form_submit');
  

  return $form;
}


function student_application_consent_validate ($form, &$form_state) {
  // make sure we filled it out properly
  if ($form_state['values']['terms_and_conditions_agreement'] != "I AGREE") {
    form_set_error('terms_and_conditions_agreement', "You must type 'I AGREE' (all caps) to indicate acceptance of the terms and conditions.");
  }
}




function student_application_consent_form_submit($form, &$form_state) {
  // if we got here, the form validated, so rewrite our acceptance into acceptance
  $form_state['values']['terms_and_conditions_agreement'] = 1;

  // insert if new, otherwise update
  if ($form_state['values']['application_id'] > 0) {
    student_application_consent_update($form_state['values']);
  } else {
    student_application_consent_insert($form_state['values']);
  }

  drupal_set_message("Your application has been saved successfully.  You can return at any point to update or submit it.", "status");
  global $user;
  if ($user->uid == $form_state['values']['uid']) {
    $form_state['redirect'] = 'user/student/application';
  } else {
    $u = eto_user_load($form_state['values']['uid']);
    $form_state['redirect'] = 'users/' . $u->name . '/student/application';
  }

}


function student_application_consent_load($uid) {
  if (! is_numeric($uid)) {
    if (is_object($uid)) {
      $uid = $uid->uid;
    } else {
      $u = eto_user_load($uid);
      $uid = $u->uid;
    }
  }
  $result = db_query("SELECT * FROM {eto_student_application} WHERE uid = %d",
		     $uid);
  return db_fetch_object($result);
}


function student_application_consent_insert($n) {
  global $user;

  $now = time();

  db_query("INSERT INTO {eto_student_application}
               (terms_and_conditions_agreement, uid, created, changed)
            VALUES
               (%d, %d, %d, %d)",
	   $n['terms_and_conditions_agreement'], $n['uid'], $now, $now
	   );

  watchdog('applications', 
	   "@user_name inserted a new student application (consent)",
	   array("@user_name" => $user->name),
	   WATCHDOG_NOTICE,
	   l('view', 'user/' . $n['uid'] . 'student/application/consent')
	   );
}

function student_application_consent_update($n) {
  global $user;

  db_query("UPDATE {eto_student_application} SET
               terms_and_conditions_agreement = %d,
               uid = %d,
               status = %d,
               changed = %d
            WHERE
               application_id = %s",
	   $n['terms_and_conditions_agreement'], $n['uid'], $n['status'], time(),
	   $n['application_id']);


  watchdog('applications', 
	   "@user_name updated a new student application (consent)",
	   array("@user_name" => $user->name),
	   WATCHDOG_NOTICE,
	   l('view', 'user/' . $n['uid'] . 'student/application/consent')
	   );

}



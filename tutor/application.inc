<?php

function _tutor_validate_permissions($uid) {
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
    if ( ! ( ($user->uid == $uid) || user_access("administer tutors") ) ) {
      return 0;
    } else {
      return 1;
    }
  }
}

function _tutor_load_application_part($uid, $part) {
  if (! is_numeric($uid)) {
    if (is_object($uid)) {
      $uid = $uid->uid;
    } else {
      $u = eto_user_load($uid);
      $uid = $u->uid;
    }
  }

  if ($part == "consent") {
    return tutor_application_consent_load($uid);
  } else if ($part == "profile") {
    module_load_include("inc", "tutor", "profile");
    return tutor_profile_load($uid);
  } else if ($part == "contact") {
    module_load_include("inc", "eto_admin", "contact");
    return eto_contact_load($uid, 'CONTACT');
  } else if ($part == "computer") {
    module_load_include("inc", "tutor", "profile");
    return tutor_computer_load($uid);
  } else {
    return 0;
  }
}

function _tutor_validate_application_part($node, $part) {
  if ($part == "consent") {
    return tutor_application_consent_validate($node);
  } else if ($part == "contact") {    
    return 0; // if it was saved, it's validated
  } else if ($part == "profile") {
    module_load_include("inc", "tutor", "profile");
    return tutor_profile_validate($node);
  } else if ($part == "computer") {
    module_load_include("inc", "tutor", "profile");
    return tutor_computer_validate($node);
  } else {
    return 1;
  }
}


/***************************************************************************
 *
 * application status page
 *
 ***************************************************************************/

function tutor_application($uid = "") {
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

  if (! _tutor_validate_permissions($uid)) {
    return "Permission denied.";
  }
  // else

  $app = _tutor_load_application_part($uid, 'consent');

  if ($app && ($app->status > 0)) {
    if ($app->status != APP_APPROVED) {
      if ($mine) {
	if ($app->status == APP_REJECTED) {
	  $output .= "<p>Sorry, this application has been rejected.</p>";
	} else {
	  $output .= "<p>Thank you for submitting your application.  We will get back to you as soon as possible.</p>";
	}
      } else {
	$output .= "<p>This application has been submitted and is waiting for your approval.</p>";
	$parts_status = _tutor_display_application_parts ($uid, $mine);
	$output .= $parts_status['output'];
	$node = tutor_application_consent_load($uid);
	$output .= drupal_get_form('tutor_application_admin_form', $node);
	$output .= "<p>Once you have done everything above, click below to promote this user to be a tutor and send them an email.</p>";
	$output .= drupal_get_form('tutor_application_approval_form', $node);
      }
    } else if ($app->status == APP_APPROVED) {
      // already a tutor
      if ($mine) {
	$output .= "You are already a tutor!";
      } else {
	$output .= "<p><strong>This user is already a tutor.</strong>  Their application is included below if you wish to review any pieces of it.  Please note that the contact information and biography may have been changed since when they were submitted.</p>";
	if (! in_array('Tutor', array_values($target_user->roles))) {
	  drupal_set_message("<strong>Warning:</strong> This user does not have the 'Tutor' role set, so they are not fully promoted as a tutor. " . l('Click here to add the tutor role.', 'user/' . $uid . '/edit'), "warning");
	}
	$parts_status = _tutor_display_application_parts ($uid, $mine);
	$output .= $parts_status['output'];
      }
    }
  } else {
    if ($mine) { 
      $output .= "Please do not complete this application unless you meet the following technical requirements:";
      $output .= "<ol><li>You currently have a computer connected to a functional webcam.</li><li>You have a skype account and know how to make skype video calls.</li><li>You have high-speed internet access, and a quiet place from which tutor, preferably with an ethernet, not wireless connection.</li></ol>";
      $output .= "<p>When you have completed all parts of the application process, you may submit your application.</p>";
    } else {
      $output .= "<p>Application status:</p>";
    }
    
    $parts_status = _tutor_display_application_parts ($uid, $mine);
    $output .= $parts_status['output'];
    
    if ($parts_status['exists'] == $parts_status['total']) {
      // validate our nodes to make sure the important stuff is handled // FIXME
      if ($parts_status['exists'] == $parts_status['completed']) {
	if ($mine) {
	  $output .= "Your application is complete!  Please review it and make any final changes.  After you submit it, you will not be able to change it again.";
	} else {
	  $output .= $target_user->name . "'s application is complete.  If you wish to submit it for them, please click the submit button below.";
	}
	$node = tutor_application_consent_load($uid);
	$output .= drupal_get_form('tutor_application_submit_form', $node);
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

function _tutor_display_application_parts ($uid, $mine = 0) {
  $target_user = eto_user_load($uid);
  $uid = $target_user->uid;

  $status = array();

  $parts = array('contact' => 'Contact Information',
		 'computer' => 'Computer Information',
		 'profile' => 'Biographical Information',
		 'consent' => 'Self Description',
		 );

  $status['output'] .= "\n\n<table>\n";

  
  foreach ($parts as $key => $text) {
    
    $status['total'] ++;
    
    $data = _tutor_load_application_part($target_user->uid, $key);
    if ($data) {
      $status['exists'] ++;
      $messages = _tutor_validate_application_part($data, $key);
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
      $link = "user/tutor/application/$key";
    } else {
      $link = "users/" . $target_user->name . "/tutor/application/$key";
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

function tutor_application_submit_form(&$form_state, $node) {
  $form['uid'] = array (
			    '#type' => 'hidden',
			    '#value' => $node->uid,
			    );
  $form['submit'] = array (
			   '#type' => 'submit',
			   '#value' => 'Submit application',
			   );
  $form['#submit'] = array('tutor_application_submit_form_submit');
  return $form;
}

function tutor_application_submit_form_submit($form, &$form_state) {
  $node = tutor_application_consent_load($form_state['values']['uid']);
  $node->status = 1;
  tutor_application_consent_update(to_array($node));
  drupal_set_message("Your application has been submitted successfully!  We will get back to you shortly.", "status");

  global $user;
  if ($user->uid == $form_state['values']['uid']) {
    $form_state['redirect'] = 'user/tutor/application';
  } else {
    $u = eto_user_load($form_state['values']['uid']);
    $form_state['redirect'] = 'users/' . $u->name . '/tutor/application';
  }
}

/***************************************************************************
 *
 * tutor approval subsection
 *
 ***************************************************************************/

function tutor_application_approval_form(&$form_state, $node) {
  $form['uid'] = array (
			    '#type' => 'hidden',
			    '#value' => $node->uid,
			    );
  $form['submit'] = array (
			   '#type' => 'submit',
			   '#value' => 'Approve this Tutor',
			   );
  $form['#submit'] = array('tutor_application_approval_form_submit');
  return $form;
}

function tutor_application_approval_form_submit($form, &$form_state) {
  $node = tutor_application_consent_load($form_state['values']['uid']);
  $node->status = APP_APPROVED;
  $u = eto_user_load($form_state['values']['uid']);

  $params['username'] = $u->name;
  $params['full_name'] = $u->eto_profile->first_name . " " . $u->eto_profile->last_name;

  tutor_application_consent_update(to_array($node));

  drupal_mail("eto_admin", 
	      "new_tutor",
	      $u->mail,
	      language_default(), 
	      $params, 
	      variable_get('site_mail', 'admin@elitetutorsonline.com'), 
	      TRUE);

  drupal_set_message("Tutor's application is approved and the tutor has been notified.", "status");
  drupal_set_message("To finish adding them as a tutor you *must* select the 'Tutor' role below and save their account.", "error");
  $form_state['redirect'] = 'user/' . $u->uid . '/edit';
}


/***************************************************************************
 *
 * contact subsection
 *
 ***************************************************************************/

function tutor_application_contact ($name = "") {
  $output = "";
  global $user;

  if (!$name) {
    $name = $user->name;
  }
  $u = eto_user_load(array('name' => $name));
  if (! _tutor_validate_permissions($u->uid)) {
    return "Permission denied.";
  } 
  // else

  module_load_include('inc', 'eto_admin', 'contact');
  $node = eto_contact_load($u->uid, 'CONTACT');

  $output .= variable_get('eto_contact_form_help_text', '<h3>Please fill out the contact information below:</h3>');

  global $user;
  if ($user->uid == $u->uid) {
    $redirect = 'user/tutor/application'; // fixme
  } else {
    $redirect = 'users/' . $u->name . '/tutor/application';
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

function tutor_application_profile ($name = "") {
  $output = "";
  global $user;

  if (!$name) {
    $name = $user->name;
  }
  $u = eto_user_load(array('name' => $name));
  if (! _tutor_validate_permissions($u->uid)) {
    return "Permission denied.";
  } 
  // else

  module_load_include('inc', 'tutor', 'profile');
  $node = tutor_profile_load($u->uid);


  $output .= variable_get('eto_tutor_profile_form_help_text', '<h3>Please fill out the information below:</h3>');

  if ($node) {
    $messages = _tutor_validate_application_part($node, 'profile');
    if ($messages) {
      drupal_set_message(implode('<br>', $messages), "warning");
    }
  }

  $output .= drupal_get_form('tutor_profile_form', $node, array('uid' => $u->uid));

  return $output;
}

/***************************************************************************
 *
 * computer subsection
 *
 ***************************************************************************/

function tutor_application_computer ($name = "") {
  $output = "";
  global $user;

  if (!$name) {
    $name = $user->name;
  }
  $u = eto_user_load(array('name' => $name));
  if (! _tutor_validate_permissions($u->uid)) {
    return "Permission denied.";
  } 
  // else

  module_load_include('inc', 'tutor', 'profile');
  $node = tutor_computer_load($u->uid);


  $output .= variable_get('eto_tutor_computer_form_help_text', '<h3>Please fill out the information below:</h3>');

  if ($node) {
    $messages = _tutor_validate_application_part($node, 'computer');
    if ($messages) {
      $output .= "<div id='validate-messages'>";
      $output .= implode('<br>', $messages);
      $output .= "</div>";
    }
  }

  $output .= drupal_get_form('tutor_computer_form', $node, array('uid' => $u->uid));

  return $output;
}

/***************************************************************************
 *
 * consent (self description) subsection
 *
 ***************************************************************************/

function tutor_application_consent($name = "") {
  $output = "";
  global $user;

  if (!$name) {
    $name = $user->name;
  }
  $u = eto_user_load(array('name' => $name));
  if (! _tutor_validate_permissions($u->uid)) {
    return "Permission denied.";
  }
  // else
  
  $node = tutor_application_consent_load($u->uid);

  $output .= variable_get('eto_tutor_consent_form_help_text', '<h3>Please fill out the information below:</h3>');

  if ($node) {
    $messages = _tutor_validate_application_part($node, 'consent');
    if ($messages) {
      $output .= "<div id='validate-messages'>";
      $output .= implode('<br>', $messages);
      $output .= "</div>";
    }
  }

  $output .= drupal_get_form('tutor_application_consent_form', $node, array('uid' => $u->uid));

  return $output;
}

/**
 * Implementation of hook_form
 */

function tutor_application_consent_form(&$form_state, $node, $vars) {


  $form['application_id'] = array (
				   '#type' => 'hidden',
				   '#value' => ($node->application_id ? $node->application_id : 0),
				   );
  $form['uid'] = array (
			    '#type' => 'hidden',
			    '#value' => ($node->uid ? $node->uid : $vars['uid']),
			    );

  $form['self_description'] = array (
				     '#type' => 'textarea',
				     '#title' => 'Please use the space below to write a brief message introducing yourself to potential tutoring clients',
				     '#required' => TRUE,
				     '#default_value' => $node->self_description,
				     );

  $form['new_photo_id_fid'] = array (
					       '#type' => 'image_upload_element',
					       '#title' => 'A copy of your photo ID',
					       '#description' => 'File must be 2MB or less.',
					       '#default_value' => ($node->new_photo_id_fid
								    ? eto_image_load($node->new_photo_id_fid)
								    : eto_image_load($node->photo_id_fid)
								    ),
					       );
  $form['new_resume_fid'] = array (
					       '#type' => 'upload_element',
					       '#title' => 'Your Resume',
					       '#description' => 'File must be 2MB or less.',
					       '#default_value' => ($node->new_resume_fid
								    ? eto_image_load($node->new_resume_fid)
								    : eto_image_load($node->resume_fid)
								    ),
					       );


  $form['save'] = array (
			   '#type' => 'submit',
			   '#value' => 'Save this section',
			   );

  $form['#submit'] = array('tutor_application_consent_form_submit');
  

  return $form;
}


function tutor_application_consent_validate ($node) {
  // make sure we filled it out properly
  if (strlen($node->self_description) > 0) {
    return 0;
  } else {
    return array("You need to fill out your self description");
  }

}

function sb_file_presave($item, $path = "") {
  $destination = file_directory_path() . ($path ? "/" . $path : "");

  $fid = upload_element_save($item, $destination, FILE_EXISTS_RENAME);
  return $fid;
}


function tutor_application_consent_form_submit($form, &$form_state) {

  if ($form_state['values']['new_photo_id_fid']) {
    $form_state['values']['photo_id_fid'] = sb_file_presave($form_state['values']['new_photo_id_fid'], "tutor-files");
  }
  if ($form_state['values']['new_resume_fid']) {
    $form_state['values']['resume_fid'] = sb_file_presave($form_state['values']['new_resume_fid'], "tutor-files");
  }

  // insert if new, otherwise update
  if ($form_state['values']['application_id'] > 0) {
    tutor_application_consent_update($form_state['values']);
  } else {
    tutor_application_consent_insert($form_state['values']);
  }

  drupal_set_message("Your application has been saved successfully.  You can return at any point to update or submit it.", "status");
  global $user;
  if ($user->uid == $form_state['values']['uid']) {
    $form_state['redirect'] = 'user/tutor/application';
  } else {
    $u = eto_user_load($form_state['values']['uid']);
    $form_state['redirect'] = 'users/' . $u->name . '/tutor/application';
  }

}


function tutor_application_consent_load($uid) {
  if (! is_numeric($uid)) {
    if (is_object($uid)) {
      $uid = $uid->uid;
    } else {
      $u = eto_user_load($uid);
      $uid = $u->uid;
    }
  }
  $result = db_query("SELECT * FROM {eto_tutor_application} WHERE uid = %d",
		     $uid);
  $obj = db_fetch_object($result);
  $obj->todo_list = unserialize($obj->todo_list);
  return $obj;
}


function tutor_application_consent_insert($n) {
  global $user;

  $now = time();

  db_query("INSERT INTO {eto_tutor_application}
               (self_description, photo_id_fid, resume_fid, 
                uid, created, changed)
            VALUES
               ('%s', %d, %d,
                %d, %d, %d)",

	   $n['self_description'], $n['photo_id_fid'], $n['resume_fid'],
	   $n['uid'], $now, $now
	   );

  watchdog('applications', 
	   "@user_name inserted a new tutor application (consent)",
	   array("@user_name" => $user->name),
	   WATCHDOG_NOTICE,
	   l('view', 'user/' . $n['uid'] . 'tutor/application/consent')
	   );
}

function tutor_application_consent_update($n) {
  global $user;

  db_query("UPDATE {eto_tutor_application} SET
               self_description = '%s',
               photo_id_fid = %d,
               resume_fid = %d,
               uid = %d,
               status = %d,
               changed = %d
            WHERE
               application_id = %s",
	   $n['self_description'], $n['photo_id_fid'], $n['resume_fid'], 
	   $n['uid'], $n['status'], time(),
	   $n['application_id']);


  watchdog('applications', 
	   "@user_name updated a new tutor application (consent)",
	   array("@user_name" => $user->name),
	   WATCHDOG_NOTICE,
	   l('view', 'user/' . $n['uid'] . 'tutor/application/consent')
	   );

}


/***************************************************************************
 *
 * admin section
 *
 ***************************************************************************/

function tutor_application_admin_form(&$form_state, $node) {

  $form['application_id'] = array ('#type' => 'hidden', '#value' => $node->application_id);

  $status_options = app_status("", TRUE);
  $form['status'] = array (
			   '#type' => 'select',
			   '#options' => $status_options,
			   '#title' => 'Application Status',
			   '#default_value' => $node->status,
			   );
  $todo_options = app_todo();

  $form['todo_list'] = array (
			      '#type' => 'checkboxes',
			      '#options' => $todo_options,
			      '#title' => 'Administrative To Do List',
			      '#default_value' => ($node->todo_list ? $node->todo_list : array()),
			      );

  $form['save'] = array (
			   '#type' => 'submit',
			   '#value' => 'Save',
			   );

  $form['#submit'] = array('tutor_application_admin_form_submit');

  return $form;
}

function tutor_application_admin_form_submit($form, &$form_state) {
  // we're only writing a subset of the fields
  db_query("UPDATE {eto_tutor_application} SET
              status = %d,
              todo_list = '%s'
            WHERE
                application_id = %s",
	   $form_state['values']['status'],
	   serialize($form_state['values']['todo_list']),
	   $form_state['values']['application_id']
	   );
  drupal_set_message("Changes saved.", "success");
}
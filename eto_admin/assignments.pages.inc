<?php

/**
 * student's view of their tutors
 */

function user_tutors ($target_user = "") {
  if (!$target_user) {
    global $user;
    $target_user = $user;
  } else {
    $target_user = eto_user_load(array('name' => $target_user));
  }

  if (!$target_user) {
    drupal_not_found();
    exit;
  }

  if ($target_user->uid != $user->uid) {
    if (! user_acccess('administer the site')) {
      drupal_access_denied();
      exit;
    }
  }


  $title = "My Tutors";
  drupal_set_title($title);

  $output .= user_tutors_block($target_user);

  return $output;
}

function user_tutors_block ($target_user, $check_permissions = FALSE) {
  if (! is_object($target_user)) {
    $target_user = eto_user_load($target_user);
  }
  $output = "";

  if ($check_permissions) {
    global $user;
    if (($user->uid != $target_user->uid) && (! user_access("administer the site"))) {
      return "";
    }
  }

  $tutors = sb_assignment_load ($target_user->uid, "tutor_uid");

  $output = "<h2>My Tutors</h2>\n";
  if ($tutors) {
    $output .= "<ul>\n";
    foreach ($tutors as $username => $name) {
      $u = eto_user_load($username);
      $output .= "<li>" 
	. l("[schedule]", "users/" . $username . "/schedule") . " "
	. theme('eto_user', $u, TRUE)
	. "</li>\n";
    }
    $output .= "</ul>\n";
  } else {
    $output .= "<p>You have no tutors assigned yet.</p>\n";
  }

  return $output;

}

/**
 * tutor's view of their students
 */

function user_students ($target_user = "") {
  if (!$target_user) {
    global $user;
    $target_user = $user;
  } else {
    $target_user = eto_user_load(array('name' => $target_user));
  }

  if (!$target_user) {
    drupal_not_found();
    exit;
  }

  if ($target_user->uid != $user->uid) {
    if (! user_acccess('administer the site')) {
      drupal_access_denied();
      exit;
    }
  }

  $title = "My Students";
  drupal_set_title($title);
  $output = "<h2>$title</h2>\n";

  $students = sb_assignment_load ($target_user->uid, "student_uid");

  $schedule_url = sb_user_base($target_user) . "/schedule";
  
  $output .= l("+Add an appointment with a student", $schedule_url);

  if ($students) {
    $output .= "<ul>\n";
    foreach ($students as $uid => $name) {
      $u = eto_user_load($uid);
      $output .= "<li>" 
	. theme('eto_user', $u, TRUE)
	. "</li>\n";
    }
    $output .= "</ul>\n";
  } else {
    $output .= "<p>You have no students assigned yet.</p>\n";
  }

  return $output;
}


/**
 * admin view of tutor assignments
 */

function tutor_assignments($by = 0) {
  $output = "";

  $output .= "<h2>Tutor Assignments</h2>";

  $tutor_string = "List assignments by tutor";
  $tutor_url = 'eto/admin/assignments/by-tutor';
  $student_string = "List assignments by student";
  $student_url = 'eto/admin/assignments/by-student';

  $by_tutor = 1;

  if ($by == "by-tutor") {
    $users = sb_tutor_array();
    $fetch = "student";
    $title = "List of students by tutor";
    $head = array ('', 'Tutor', 'Students');
    $student_string = l($student_string, $student_url);
    $tutor_string = "<strong>$tutor_string</strong>\n";
  } else {
    $users = sb_student_array();
    $fetch = "tutor";
    $title = "List of tutors by student";
    $head = array ('', 'Student', 'Tutors');
    $tutor_string = l($tutor_string, $tutor_url);
    $student_string = "<strong>$student_string</strong>\n";
  }

  drupal_set_title($title);
  $output .= "<h2>$title</h2>\n";

  $output .= "<center>" . $student_string . " | " . $tutor_string . "</center>\n";

  $rows = array();
  foreach ($users as $name => $fullname) {
    $u = eto_user_load($name);
    $rows[] = theme('assignment_row', $u, $fetch, TRUE);
  }

  $output .= theme('table', $head, $rows);


  return $output;

}


/******************************************************************************
 ******************************************************************************
 **
 ** add/remove tutors/students
 **
 ******************************************************************************
 ******************************************************************************/

/*
 * add a $type to $username
 */

function tutor_assignment_add($username, $type) {
  $type = rtrim($type, "s");

  $title = "Add a $type to $username";
  drupal_set_title($title);
  $output = "<h2>$title</h2>";

  $user = eto_user_load(array('name' => $username));

  $node = (object) array ('uid' => $user->uid,
			  'type' => $type);

  $output .= drupal_get_form('add_assignment_form', $node);

  return $output;
}

function add_assignment_form(&$form_state, $node) {
  $form = array();

  $form['type'] = array ('#type' => 'hidden', '#value' => $node->type);  
  $form['uid'] = array ('#type' => 'hidden', '#value' => $node->uid);

  if ($node->type == "student") {
    $form['key_field'] = array ('#type' => 'hidden', '#value' => 'tutor_uid');
    $form['target_field'] = array ('#type' => 'hidden', '#value' => 'student_uid');
    $options = sb_student_array('--Select a Student--');
  } else {
    $form['key_field'] = array ('#type' => 'hidden', '#value' => 'student_uid');
    $form['target_field'] = array ('#type' => 'hidden', '#value' => 'tutor_uid');
    $options = sb_tutor_array('--Select a Tutor--');
  }
  $form['selected_uid'] = array (
				 '#type' => 'select',
				 '#options' => $options,
				 '#title' => 'Select',
				 '#default_value' => ($node->selected_uid ? $node->selected_uid : 0),
				 );
  $form['submit'] = array (
			   '#type' => 'submit',
			   '#value' => 'Add this ' . $node->type,
			   );

  $form['#submit'][] = 'add_assignment_form_submit_handler';

  return $form;
}

function add_assignment_form_submit_handler ($form, &$form_state) {
  // only one choice: insert
  $selected = eto_user_load($form_state['values']['selected_uid']);
  $user = eto_user_load($form_state['values']['uid']);

  $form_state['values']['selected_uid'] = $selected->uid;
  sb_assignment_insert ( (object) $form_state['values'] );


  drupal_set_message(ucwords($form_state['values']['type']) . " '" 
		     . l($selected->name, "users/" . $selected->uid) 
		     . "' added to " 
		     . l($user->name, "users/" . $user->uid),
		     "success");
  $form_state['redirect'] = 'eto/admin/assignments';
}

function sb_assignment_insert ($node) {

  // we're expecting key field and dtarget field set so we know what to insert

  db_query("INSERT INTO {eto_assignments}
            (" . $node->key_field . ", " . $node->target_field . ")
            VALUES
            (%d, %d)",
	   $node->uid, $node->selected_uid);

}


/*
 * delete a $type from $username
 */

function tutor_assignment_delete($username, $type, $selected_uid) {
  $type = rtrim($type, "s");

  $select = eto_user_load($selected_uid);
  
  $title = "Delete the $type '" . $select->name . "' from $username";
  drupal_set_title($title);
  $output = "<h2>$title</h2>";

  $user = eto_user_load(array('name' => $username));

  $node = (object) array ('uid' => $user->uid,
			  'selected_uid' => $selected_uid,
			  'type' => $type);

  $output .= "Are you sure you want to delete this assignment?";
  $output .= drupal_get_form('delete_assignment_form', $node);

  return $output;
}

function delete_assignment_form(&$form_state, $node) {
  $form = array();

  $form['type'] = array ('#type' => 'hidden', '#value' => $node->type);  
  $form['uid'] = array ('#type' => 'hidden', '#value' => $node->uid);
  $form['selected_uid'] = array ('#type' => 'hidden', '#value' => $node->selected_uid);

  if ($type == "student") {
    $form['key_field'] = array ('#type' => 'hidden', '#value' => 'student_uid');
    $form['target_field'] = array ('#type' => 'hidden', '#value' => 'tutor_uid');
  } else {
    $form['key_field'] = array ('#type' => 'hidden', '#value' => 'tutor_uid');
    $form['target_field'] = array ('#type' => 'hidden', '#value' => 'student_uid');
  }
  $form['submit'] = array (
			   '#type' => 'submit',
			   '#value' => 'Yes, delete this ' . $node->type,
			   );

  $form['#submit'][] = 'delete_assignment_form_submit_handler';

  return $form;
}

function delete_assignment_form_submit_handler ($form, &$form_state) {
  // only one choice: delete
  sb_assignment_delete ( (object) $form_state['values'] );

  $selected = eto_user_load($form_state['values']['selected_uid']);
  $user = eto_user_load($form_state['values']['uid']);

  drupal_set_message(ucwords($form_state['values']['type']) . " '" 
		     . l($selected->name, "users/" . $selected->uid) 
		     . "' deleted from " 
		     . l($user->name, "users/" . $user->uid),
		     "success");
  $form_state['redirect'] = 'eto/admin/assignments';
}

function sb_assignment_delete ($node) {

  // we're expecting key field and dtarget field set so we know what to delete

  db_query("DELETE FROM {eto_assignments}
            WHERE " . $node->key_field . " = %d AND " . $node->target_field . " = %d",
	   $node->uid, $node->selected_uid);

}
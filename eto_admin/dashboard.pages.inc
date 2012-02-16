<?php

function eto_admin_dashboard() {
  $output = "";

  $output .= "<h2>Today</h2>";
  $output .= "<ul>";
  $output .= "<li>" . l("Number of tutors teaching today: " . count_tutors_with_sessions_today(), "eto/admin/tutors-today") . "</li>";
  $output .= "<li>" . l("Number of sessions today: " . count(sessions_today()), "eto/admin/sessions-today") . "</li>";
  $output .= "</ul>";

  $output .= "<h2>Actions</h2>";
  $output .= "<ul>";
  $output .= "<li>" . l("Tutor/Student pairs who need followup: " . count(_get_followups()), "eto/admin/assignment-followups") . "</li>\n";
  $output .= "<li>" . l("Pending Tutors: " . count(tutor_applications_in_progress()), "eto/admin/pending-tutors") . "</li>\n";
  $output .= "<li>" . l("Pending Students: " . count(student_applications_in_progress()), "eto/admin/pending-students") . "</li>\n";
  $output .= "<li>" . l("Assignments (by student)", "eto/admin/assignments/by-student") . "</li>\n";
  $output .= "<li>" . l("Assignments (by tutor)", "eto/admin/assignments/by-tutor") . "</li>\n";
  $output .= "<li>" . l("Adjust Billing Levels", "eto/admin/billing-levels") . "</li>\n";
  $output .= "</ul>";

  $output .= "<h2>Billing</h2>";
  $output .= "<ul>";
  $output .= "<li>" . l("Generate Reports", "eto/admin/reports") . "</li>\n";
  $output .= "<li>" . l("Logged Sessions", "eto/admin/logged-sessions") . "</li>\n";
  $output .= "</ul>";


  $output .= "<h2>Site Status</h2>";
  $output .= "<ul>";
  $output .= "<li>" . l("Number of tutors: " . count(sb_tutor_array()), "eto/admin/tutor-list") . "</li>";
  $output .= "<li>" . l("Number of students: " . count(sb_student_array()), "eto/admin/student-list") . "</li>";
  $output .= "<li> " . l("View all accounts on the site", "admin/user/user") . "</li>";
  $output .= "</ul>";

  $output .= "<h2>Site Configuration</h2>";
  $output .= "<ul>";
  $output .= "<li>" . l("Edit the Terms and Conditions", "admin/build/block/configure/eto_admin/terms-and-conditions") . "</li>";
  $output .= "<li>" . l("Edit the subjects tutors can teach", "admin/content/taxonomy/2") . "</li>";
  $output .= "</ul>";

  return $output;
}

function eto_pending_tutors() {
  $output = "";

  $output .= "<h2>Pending Tutors</h2>";

  $tutors = tutor_applications_in_progress();

  if ($tutors) {
    $output .= "<dl>";
    foreach ($tutors as $t) {
      $user = eto_user_load($t->uid);
      $output .= "<dt>" . l("[view] " . $user->name, "users/" . $user->name . "/tutor/application") . " (Status: " . app_status($t->status) . ")</dt>";
      $output .= "<dd>";
      $options = app_todo();
      foreach ($options as $key => $value) {
	if ($t->todo_list[$key]) {
	  $output .= theme('eto_green', "X $value");
	} else {
	  $output .= theme('eto_red', "- $value");
	}
	$output .= "<br>";
      }
      $output .= "</dd>\n";
    }
    $output .= "</dl>";
  } else {
    $output .= "<p><em>There are no tutor applications that need action from you.</em></p>\n";
  }

  return $output;
}

function eto_pending_students() {
  $output = "";

  $output .= "<h2>Pending Students</h2>";

  $students = student_applications_in_progress();

  if ($students) {
    $output .= "<dl>";
    foreach ($students as $t) {
      $user = eto_user_load($t->uid);
      $output .= "<dt>" . l("[view] " . $user->name, "users/" . $user->name . "/student/application") . " (Status: " . app_status($t->status) . ")</dt>";
      /*
      $output .= "<dd>";
      $options = app_todo();
      foreach ($options as $key => $value) {
	if ($t->todo_list[$key]) {
	  $output .= theme('eto_green', "X $value");
	} else {
	  $output .= theme('eto_red', "- $value");
	}
	$output .= "<br>";
      }
      $output .= "</dd>\n";
      */
    }
    $output .= "</dl>";
  } else {
    $output .= "<p><em>There are no student applications that need action from you.</em></p>\n";
  }

  return $output;
}

function eto_tutors_today() {
  $tutors = tutors_with_sessions_today();

  $output = "<h2>Tutors with sessions today</h2>";

  if ($tutors) {
    $output .= "<ul>";
    foreach ($tutors as $t) {
      $u = eto_user_load($t->uid);
      $start = date_make_date($t->start_datetime_utc, "UTC");
      $end = date_make_date($t->end_datetime_utc, "UTC");
      date_timezone_set($start, timezone_open(user_tz()));
      date_timezone_set($end, timezone_open(user_tz()));
      $output .= "<li>" . l($u->name, "users/" . $u->name) . ": " . date_format($start, TIME_FORMAT_ETO_STANDARD) . " - " . date_format($end, TIME_FORMAT_ETO_STANDARD) . "</li>";
    }
    $output .= "</ul>\n";
  } else {
    $output .= "<p><em>No tutors have sessions today.</em></p>\n";
  }
  return $output;  
}


function eto_sessions_today() {
  $sessions = sessions_today();

  $output = "<h2>Sessions today</h2>";

  if ($sessions) {
    $output .= "<ul>";
    foreach ($sessions as $s) {
      $u = eto_user_load($s->tutor_uid);
      $start = date_make_date($s->start_datetime_utc, "UTC");
      $end = date_make_date($s->end_datetime_utc, "UTC");
      date_timezone_set($start, timezone_open(user_tz()));
      date_timezone_set($end, timezone_open(user_tz()));
      $participants = sb_appointment_participants_load($s->appointment_id, "names");
      $output .= "<li>" 
	. l("[view] " . $s->title, appointment_url("", $s->appointment_id, $s->appointment_event_id)) . " "
	. "from " . date_format($start, TIME_FORMAT_ETO_STANDARD) . " - " . date_format($end, TIME_FORMAT_ETO_STANDARD) . ". "
	. "tutor: " . l($u->name, "users/" . $u->name)  . ". "
	. "students: " . implode(", ", format_participant_array($participants))
	. "</li>";
    }
    $output .= "</ul>\n";
  } else {
    $output .= "<p><em>No sessions today.</em></p>\n";
  }
  return $output;  
}


function eto_tutor_list() {
  $output = "<H2>Tutor List</h2>";

  $tutors = sb_tutor_array();
  if ($tutors) {
    $output .= "<ul>";
    foreach ($tutors as $username => $name) {
      $output .= "<li>" . l($name, "users/" . $username) . "</li>";
    }
    $output .= "</ul>";
  } else {
    $output .= "<p><em>No tutors found.</em></p>\n";
  }

  return $output;
}

function eto_student_list() {
  $output = "<H2>Student List</h2>";

  $students = sb_student_array();
  if ($students) {
    $output .= "<ul>";
    foreach ($students as $username => $name) {
      $output .= "<li>" . l($name, "users/" . $username) . "</li>";
    }
    $output .= "</ul>";
  } else {
    $output .= "<p><em>No students found.</em></p>\n";
  }

  return $output;
}


function _get_followups() {
  $result = db_query("SELECT * FROM {eto_assignments} WHERE session_count >= 1 AND followup_complete = 0");
  $pairs = array();
  while ($n = db_fetch_object($result)) {
    $pairs[] = $n;
  }
  return $pairs;
}


function assignment_followups() {
  $output = "<h2>Tutor/Student Pair Followups</h2>";

  $pairs = _get_followups();

  $rows = array();
  $th = array("Tutor", "Student", "Sessions", "Mark as done");

  if ($pairs) {
    foreach ($pairs as $pair) {
      $tutor = eto_user_load($pair->tutor_uid);
      $student = eto_user_load($pair->student_uid);

      $rows[] = array (
		     l(theme('eto_user', $tutor), "users/" . $tutor->name),
		     l(theme('eto_user', $student), "users/" . $student->name),
		     $pair->session_count,
		     drupal_get_form('eto_followup_form', $pair));
    }
    if ($rows) {
      $output .= theme('table', $th, $rows);
    }
  } else {
    $output .= "<p>No tutor/student pairs are currently sitting at exactly one session!  Please check back later.</p>";
  }

  return $output;
}


function eto_followup_form($form_state, $node) {
  $form = array();

  $form['id'] = array (
		       '#type' => 'hidden',
		       '#value' => $node->id);
  $form['submit'] = array (
			   '#type' => 'submit',
			   '#value' => 'Followup Completed',
			   );
  $form['#submit'] = array('eto_followup_form_submit_handler');
  return $form;
}

function eto_followup_form_submit_handler($form, &$form_state) {
  db_query("UPDATE {eto_assignments} SET followup_complete = 1 WHERE id = %d", $form_state['values']['id']);
  drupal_set_message('Followup marked completed.', "success");
}
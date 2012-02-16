<?php

/************************************************************************************
 ************************************************************************************
 **
 ** Session log displaying
 **
 ************************************************************************************
 ************************************************************************************/

function sb_session_log_display ($arg) {
  $array = sb_parse_arg($arg);
  $appointment_id = $array[0];
  $event_id = $array[1];
  $event = sb_appointment_event_load(array(
             'appointment_event_id' => $event_id));

  global $user;

  // we can only view this if we're the tutor or a student or an admin
  //
  // otherwise, access denied

  if (user_access('administer the site') || isa_participant($appointment_id, $user->uid)) {

    $output .= "<h2>Session Log</h2>";
    drupal_set_title("Session Log");

    if (user_access('administer the site') || user_access('is a tutor')) {
      $tutor = 1;
    } else {
      $tutor = 0;
    }

    $log = sb_session_log_load($event_id, "event");

    $start = theme('user_date', $log->start_datetime_utc, $user);
    $date = date_format($start, DATE_FORMAT_ETO_LONG);
    $time = date_format($start, TIME_FORMAT_ETO_STANDARD);

    $output .= theme('eto_basic_field', "Title", $log->title);
    $output .= theme('eto_basic_field', 'Date', $date . " at " . $time);

    $output .= "\n\n<p></p>\n\n";

    if ($tutor && ($log->duration_original != $log->duration_final)) {
      $output .= theme('eto_basic_field', 'Scheduled Duration', $log->duration_original);
    }
    $output .= theme('eto_basic_field', 'Session Duration', $log->duration_final);
    if ($tutor && ($log->hourly_rate_original != $log->hourly_rate_final)) {
      $output .= theme('eto_basic_field', 'Scheduled Hourly Rate', theme('sb_rate', $log->hourly_rate_original, "$") . "/hour");
    }
    $output .= theme('eto_basic_field', 'Session Hourly Rate', theme('sb_rate', $log->hourly_rate_final, "$") . "/hour");
    $output .= theme('eto_basic_field', "Session Cost", theme('sb_rate', $log->total_cost, "$"));

    $output .= "\n\n<p></p>\n\n";

    if (user_access("administer the site")) {
      $output .= theme('eto_basic_field', 'Tutor Split', $log->tutor_split . "%");
      $output .= "\n\n<p></p>\n\n";
    }

    $tutor = get_session_log_tutor($log->id);
    $output .= theme('eto_basic_field', "Tutor", l($tutor->name, 'users/' . $tutor->name));

    $students = get_session_log_students($log->id);
    $sa = array();
    foreach ($students as $s) {
      $sa[] = l($s->name, 'users/' . $s->name);
    }
    $output .= theme('eto_basic_field', 'Students', implode(", ", $sa));

    $term = taxonomy_get_term($log->subject_tid);
    $output .= theme('eto_basic_field', 'Subject', ($term ? $term->name : "Unspecified"));

    $output .= "\n\n<p></p>\n\n";

    if ($tutor && $log->private_notes) {
      $output .= theme('meeting_notes', check_markup($log->private_notes, 1), "Private Notes");
    }
    if ($log->public_notes) {
      $output .= theme('meeting_notes', check_markup($log->public_notes, 1), "Notes");
      $output .= "\n\n<p></p>\n\n";
    }

    $submitted = date_make_date($log->date_submitted_utc);
    date_timezone_set($submitted, timezone_open(user_tz()));

    $output .= "<em>Session log submitted on " . date_format($submitted, DATE_FORMAT_ETO_LONG) . " at " . date_format($submitted, TIME_FORMAT_ETO_STANDARD) . "</em><br>";

  } else {
    drupal_access_denied();
    exit;
  }

  return $output;
}


/************************************************************************************
 ************************************************************************************
 **
 ** User control panel session log display
 **
 ************************************************************************************
 ************************************************************************************/


function session_log_history($target_user = "") {
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
    if (! user_access('administer the site')) {
      drupal_access_denied();
      exit;
    }
    $title = "Session History for " . $target_user->name;
  } else {
    $title = "My Session History";
  }

  if (user_access("is a tutor", $target_user)) {
    $label = "Students";
  } else {
    $label = "Tutor";
  }

  $head = array(array('data' => 'Title', 'field' => 'sa.title'),
		array('data' => 'Date', 'field' => 'sae.start_datetime_utc', 'sort' => 'desc'),
		array('data' => 'Subject', 'field' => 'td.name'),
		array('data' => 'Duration', 'field' => 'esl.duration_final'),
		array('data' => 'Total Cost', 'field' => 'esl.total_cost'),
		$label,
		"Notes");

  if (user_access("is a tutor", $target_user)) {
    $ids = get_session_logs ($target_user->uid, "sa.tutor_uid", tablesort_sql($head));
  } else {
    $ids = get_session_logs ($target_user->uid, "sap.participant_uid", tablesort_sql($head));
  }

  drupal_set_title($title);

  if (user_access("is a tutor", $target_user)) {
    $output .= session_log_display_block_delinquent($target_user);
  }

  $output .= "<h2>$title</h2>";

  if ($ids) {

    $rows = array();
    $count = 0;
    foreach ($ids as $id) {
      $log = sb_session_log_load($id, "event");
      if ($log) {
	$event = sb_appointment_event_load(array(
                   'appointment_event_id' => $id));
	$date = theme('user_date', $log->start_datetime_utc, $target_user);
	// different fields for students and tutors
	$rows[$count][] = $log->title;
	$rows[$count][] = date_format($date, DATE_FORMAT_ETO_LONG) . " at " . date_format($date, TIME_FORMAT_ETO_STANDARD);
	$term = taxonomy_get_term($log->subject_tid);
	$rows[$count][] = ($term ? $term->name : "Unspecified");
	$rows[$count][] = $log->duration_final . " min.";
	$rows[$count][] = theme('sb_rate', $log->total_cost, "$");
	if (user_access("is a tutor", $target_user)) {
	  $students = get_session_log_students($log->id);
	  $sa = array();
	  foreach ($students as $s) {
	    $sa[] = l($s->name, 'users/' . $s->name);
	  }
	  $rows[$count][] = implode(", ", $sa);
	} else {
	  $t = get_session_log_tutor($log->id);
	  $rows[$count][] = l($t->name, 'users/' . $t->name);
	}
	$rows[$count][] = l("[view]",
			    appointment_url("", $event->appointment_id, $event->appointment_event_id, "log"));
	$count ++;
      }
    }


    $output .= theme('table', $head, $rows);
  } else {
    $output .= "<p>You don't have any session logs yet.</p><p>Perhaps you haven't finished any tutoring sessions, or the tutor has not logged the sessions yet.</p>\n";
  }
  return $output;
}



/************************************************************************************
 ************************************************************************************
 **
 ** Session log writing
 **
 ************************************************************************************
 ************************************************************************************/

function sb_session_log_edit($arg) {
  $array = sb_parse_arg($arg);
  $event_id = $array[1];
  $event = sb_appointment_event_load(array(
             'appointment_event_id' => $event_id));
  $output = "";

  // see if we can get a log for this; if so, we can only edit if we're an admin
  $log = sb_session_log_load($event_id, "event");
  if ( ($log) && (! user_access("administer the site") ) ) {
    $output .= "This session has already been logged, you cannot submit a second log for a session.";
  } else {

    if ($log) {
      $title = "Edit Session Log";
    } else {
      $title = "Create Session Log";
      $log = (object) array();
      $log = $event;
    }

    drupal_set_title($title);

    $output .= "<h2>$title</h2>";

    // deliberately comparing versus the start time here in case a session ends early
    $now = date_make_date(time(), 'UTC');
    $start = date_make_date($event->start_datetime_utc, 'UTC');
    if ($start > $now) {
      $output .= "<p>You can't submit a log for a session that hasn't happened yet.</p>";
    } else {
      $output .= drupal_get_form('sb_create_session_log_form', $log);
    }
  }

  return $output;
}


function sb_create_session_log_form (&$form_state, $node) {
  $form = array();
  ahah_helper_register($form, $form_state);

  $form['appointment_event_id'] = array ('#type' => 'hidden', '#value' => $node->appointment_event_id);

  if (isset($form_state['storage']['rates_block']['duration_override'])) {
    $default['duration'] = $form_state['storage']['rates_block']['duration_override'];
  } else {
    if ($node->duration_final) {
      $default['duration'] = $node->duration_final;
    } else if ($node->duration_original) {
      $default['duration'] = $node->duration_original;
    } else if ($node->duration) {
      $default['duration'] = $node->duration;
    }
  }
  if (isset($form_state['storage']['rates_block']['rate_override'])) {
    $default['rate'] = sb_rate_validate($form_state['storage']['rates_block']['rate_override']);
  } else {
    if ($node->hourly_rate_final) {
      $default['rate'] = $node->hourly_rate_final;
    } else if ($node->hourly_rate_original) {
      $default['rate'] = $node->hourly_rate_original;
    } else if ($node->hourly_rate_override) {
      $default['rate'] = $node->hourly_rate_override;
    } else {
      $default['rate'] = $node->hourly_rate;
    }
  }

  $start = date_make_date($node->start_datetime_utc, "UTC");
  date_timezone_set($start, timezone_open(user_tz()));
  $date = date_format($start, DATE_FORMAT_ETO_LONG);
  $time = date_format($start, TIME_FORMAT_ETO_STANDARD);

  $form['title'] = array ('#value' => "<strong>Title: </strong>" . $node->title . "<br>",
			 );
  $form['date'] = array ('#value' => "<strong>Date: </strong>" . $date . " at " . $time . "<br>",
			 );
  $form['duration'] = array ('#value' => '<strong>Scheduled Duration: </strong>' . $node->duration . "<br>",
			     );
  $form['hourly_rate'] = array ('#value' => '<strong>Scheduled Rate: </strong>$'
				. theme('sb_rate', ($node->rate_override ? $node->rate_override : $node->hourly_rate))
				. "/hour<br>",
				);
  if ($node->id) {
    $t = get_session_log_tutor($node->id);
    $tutor = eto_user_load($t->uid);
    $s_obj = get_session_log_students($node->id);
    $s = array();
    foreach ($s_obj as $o) {
      $s[] = $o->name;
    }
    $students = implode(", ", $s);
  } else {
    $tutor = eto_user_load($node->tutor_uid);
    $students = implode (", ", sb_appointment_participants_load($node->appointment_id, "names"));
  }
  $form['tutor'] = array ('#value' => "<strong>Tutor: </strong>" . $tutor->name . "<br>",
			  );
  $form['students'] = array ('#value' => "<strong>Students: </strong>"
			     . $students
			     . "<br>",
			     );
  $form['rates_block'] = array(
			 '#type' => 'fieldset',
			 '#title' => 'Rate and Duration Overrides',
			 '#description' => '<strong>Cost: $' . sprintf('%.2f', ($default['rate'] / 100) * ($default['duration'] / 60)) . "</strong>",
			 '#prefix' => '<div id="rates-block-wrapper">',
			 '#suffix' => "</div>",
			 '#tree' => TRUE,
			 );
  $form['rates_block']['duration_override'] = array ('#type' => 'textfield',
					       '#size' => 4,
					       '#title' => "Actual Duration",
					       '#field_suffix' => 'minutes',
					       '#description' => "If the actual duration was different, please put it here",
					       '#default_value' => $default['duration'],
					       '#ahah' => array(
								'event' => 'change',
								'path' => ahah_helper_path(array('rates_block')),
								'wrapper' => 'rates-block-wrapper',
								),
					       );
  $form['rates_block']['rate_override'] = array ('#type' => 'textfield',
					   '#title' => "Actual Rate",
					   '#size' => 4,
					   '#description' => "If the actual rate was different, please put it here",
					   '#field_prefix' => '$',
					   '#field_suffix' => '/hour',
					   '#default_value' => theme('sb_rate', $default['rate']),
					   '#ahah' => array(
							    'event' => 'change',
							    'path' => ahah_helper_path(array('rates_block')),
							    'wrapper' => 'rates-block-wrapper',
							    ),
					   );
  $subjects = sb_tutor_subjects_select ($node->tutor_uid, "Unspecified");
  if (count($subjects == 2)) {
    $default = array_pop(array_keys($subjects));

  } else {
    $default = 0;
  }
  if ($node->subject_tid) {
    $default = $node->subject_tid;
  }

  $form['subject_tid'] = array (
				'#type' => 'select',
				'#options' => $subjects,
				'#title' => 'Select the subject for this session',
				'#default_value' => $default,
				);

  $form['notes'] = array (
			  '#type' => 'fieldset',
			  '#title' => 'Session Notes',
			  );
  $form['notes']['private_notes'] = array ('#type' => 'textarea',
					   '#title' => 'Private Notes',
					   '#description' => 'Shared only with ETO Admins',
					   '#default_value' => ($node->private_notes ? $node->private_notes : ""),
					   );
  $form['notes']['public_notes'] = array ('#type' => 'textarea',
					   '#title' => 'Public Notes',
					   '#description' => 'Shared with the student',
					   '#default_value' => ($node->public_notes ? $node->public_notes : ""),
					   );

  $form['submit'] = array (
			   '#type' => 'submit',
			   '#value' => 'Submit this session log',
			   );
  $form['submit']['#submit'][] = 'sb_create_session_log_submit_handler';

  return $form;
}


function sb_create_session_log_submit_handler($form, &$form_state) {

  $event = sb_appointment_event_load(array(
             'appointment_event_id' => $form_state['values']['appointment_event_id']));

  // make sure this hasn't already been submitted
  if ($event->event_logged == 1) {
    drupal_set_message("This session has already been logged, you cannot submit a second log for a session.", "error");
  } else {

    // build up an object of the fields we want to write based on our form_state data
    $now = date_make_date(time(), 'UTC');
    $log = array();
    $log['appointment_event_id'] = $event->appointment_event_id;
    $log['start_datetime_utc'] = $event->start_datetime_utc;
    $log['duration_original'] = $event->duration;
    $log['title'] = $event->title;
    $log['hourly_rate_original'] = ($event->hourly_rate_override ? $event->hourly_rate_override : $event->hourly_rate);
    $log['date_submitted_utc'] = date_format($now, DATE_FORMAT_DATETIME);

    $log['duration_final'] = $form_state['values']['rates_block']['duration_override'];
    $log['hourly_rate_final'] = sb_rate_validate($form_state['values']['rates_block']['rate_override']);
    $log['subject_tid'] = $form_state['values']['subject_tid'];

    $log['total_cost'] = (int) $log['hourly_rate_final'] * ($log['duration_final'] / 60);

    $log['private_notes'] = $form_state['values']['private_notes'];
    $log['public_notes'] = $form_state['values']['public_notes'];

    // we also need to add the tutor's split for Christine's billing purposes.
    $level = sb_billing_tutor_level_load($event->tutor_uid);
    $log['tutor_split'] = $level->split;

    sb_session_log_insert( (object) $log);
    log_accepted($event->appointment_event_id);

    drupal_set_message('Your session log has been saved successfully.', 'success');

  }
  // this is annoying, why is ahah_helper not clearing storage?
  $form_state['storage'] = array();
  $form_state['redirect'] = "user";
}

function sb_session_log_insert($node) {
  $event = sb_appointment_event_load(array(
             'appointment_event_id' => $node->appointment_event_id));
  $participants = sb_appointment_participants_load($event->appointment_id);

  db_query("INSERT INTO {eto_session_log}
              (appointment_event_id, start_datetime_utc, duration_original, hourly_rate_original,
               date_submitted_utc, duration_final, hourly_rate_final, subject_tid, total_cost,
               private_notes, public_notes, title, tutor_split)
            VALUES
              (%d, '%s', %d, %d, '%s', %d, %d, %d, %d, '%s', '%s', '%s', %d)",
	   $node->appointment_event_id, $node->start_datetime_utc, $node->duration_original,
	   $node->hourly_rate_original, $node->date_submitted_utc, $node->duration_final,
	   $node->hourly_rate_final, $node->subject_tid, $node->total_cost,
	   $node->private_notes, $node->public_notes, $node->title, $node->tutor_split);
  $session_log_id = db_last_insert_id("eto_session_log", "id");

  foreach ($participants as $p) {
    db_query("INSERT INTO {eto_session_log_participants}
                (session_log_id, uid, tutor)
              VALUES
                 (%d, %d, %d)",
	     $session_log_id, $p, 0);
    // also increment the count for each participant
    increment_session_count($event->tutor_uid, $p);
  }
  // and the tutor
  db_query("INSERT INTO {eto_session_log_participants}
              (session_log_id, uid, tutor)
            VALUES
               (%d, %d, %d)",
	   $session_log_id, $event->tutor_uid, 1);

}

function increment_session_count($tutor_uid, $student_uid) {
  $pair = sb_assignment_pair_load($tutor_uid, $student_uid);
  db_query("UPDATE {eto_assignments} SET session_count = %d",
	   ($pair->session_count + 1));
}

function log_accepted($event_id) {
  db_query("UPDATE {sb_appointment_event} SET event_logged = 1 WHERE appointment_event_id = %d",
	   $event_id);
}



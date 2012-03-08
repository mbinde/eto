<?php

/************************************************************************
 ************************************************************************
 **
 ** Billing Reports
 **
 ************************************************************************
 ************************************************************************/

function eto_reports() {

  $output .= "<h1>Reports</h1>";
  $output .= drupal_get_form('eto_report_form', array());

  return $output;
}


function eto_report_form(&$form_state, $node) {
  $form = array();


  $form['date'] = array (
			 '#type' => 'fieldset',
			 '#title' => 'Date Range',
			 );

  $form['date']['start'] = array (
				  '#type' => 'date_popup',
				  '#date_format' => 'm/d/Y',
				  '#date_year_range' => '-1+1',
				  '#title' => 'Start Date',
				  '#default_value' => "",
				  '#required' => TRUE,
				  );
  $form['date']['end'] = array (
				  '#type' => 'date_popup',
				  '#date_format' => 'm/d/Y',
				  '#date_year_range' => '-1+1',
				  '#title' => 'End Date',
				  '#default_value' => "",
				  '#required' => TRUE,
				  );


  $form['target'] = array (
			   '#type' => 'fieldset',
			   '#title' => 'Target',
			   '#description' => 'Select one of the below',
			   );
  $options = sb_tutor_array('*All Tutors*');
  $form['target']['tutor'] = array (
			  '#type' => 'select',
			  '#options' => $options,
			  '#title' => 'Select a Tutor',
			  '#default_value' => 0,
			  );
  $options = sb_student_array('*All Students*');
  $form['target']['student'] = array (
			  '#type' => 'select',
			  '#options' => $options,
			  '#title' => 'Select a Student',
			  '#default_value' => 0,
			  );

  $form['submit'] = array (
			   '#type' => 'submit',
			   '#value' => 'Download Report',
			   );
  $form['#validate'][] = 'eto_report_form_validate';
  $form['#submit'][] = 'eto_report_form_submit';

  return $form;
}

function eto_report_form_validate($form, &$form_state) {
  if ( (strlen($form_state['values']['tutor']) > 1) && (strlen($form_state['values']['student']) > 1) ) {
    form_set_error('student', "Select only one of a tutor or a student");
  }
}

function eto_report_form_submit($form, &$form_state) {
  global $user;

  $start = date_make_date($form_state['values']['start']);
  date_timezone_set($start, timezone_open("UTC"));
  $end = date_make_date($form_state['values']['end']);
  date_timezone_set($end, timezone_open("UTC"));

  if (strlen($form_state['values']['tutor']) > 1) {
    $target_user = eto_user_load($form_state['values']['tutor']);
    $tutor_flag = 1;
  } else if (strlen($form_state['values']['student'] > 1)) {
    $target_user = eto_user_load($form_state['values']['student']);
    $tutor_flag = 0;
  } else {
    $tutor_flag = -1;
  }

  if ($tutor_flag == -1) {
    $result = db_query("SELECT esl.*
                        FROM {eto_session_log} AS esl
                          INNER JOIN {eto_session_log_participants} AS eslp ON eslp.session_log_id=esl.id
                        WHERE eslp.tutor = 1
                          AND start_datetime_utc BETWEEN '%s' AND '%s'
                        ORDER BY eslp.uid, start_datetime_utc
                       ",
		       date_format($start, DATE_FORMAT_DATETIME),
		       date_format($end, DATE_FORMAT_DATETIME)
		       );
  } else {
    $result = db_query("SELECT esl.*
                        FROM {eto_session_log} AS esl
                          INNER JOIN {eto_session_log_participants} AS eslp ON eslp.session_log_id=esl.id
                        WHERE eslp.uid = %d
                          AND eslp.tutor = %d
                          AND start_datetime_utc BETWEEN '%s' AND '%s'
                        ORDER BY start_datetime_utc
                       ",
		       $target_user->uid, $tutor_flag,
		       date_format($start, DATE_FORMAT_DATETIME),
		       date_format($end, DATE_FORMAT_DATETIME)
		       );
  }

  $rows = array();
  $i = 0;
  while ($row = db_fetch_object($result)) {

    $date = date_make_date($row->start_datetime_utc, "UTC");
    date_timezone_set($date, timezone_open(user_tz($user)));
    $rows[$i][] = date_format($date, "m/d/Y");
    $rows[$i][] = date_format($date, "g:ia");

    $tutor = get_session_log_tutor($row->id);
    $tutor = eto_user_load($tutor->uid);
    $rows[$i][] = theme("eto_user", $tutor);

    $students = get_session_log_students($row->id);
    $s_array = array();
    foreach ($students as $s) {
      $student = eto_user_load($s->uid);
      $s_array[] = theme("eto_user", $student);
    }
    $rows[$i][] = implode(", ", $s_array);

    $subject_term = taxonomy_get_term($row->subject_tid);
    if ($subject_term) {
      $subject = $subject_term->name;
    } else {
      $subject = "Unknown";
    }
    $rows[$i][] = $subject;

    $rows[$i][] = $row->duration_original;
    $rows[$i][] = $row->duration_final;

    $rows[$i][] = $row->hourly_rate_original / 100;
    $rows[$i][] = $row->hourly_rate_final / 100;

    $rows[$i][] = $row->total_cost / 100;

    $rows[$i][] = $row->tutor_split;

    $rows[$i][] = ($row->total_cost * $row->tutor_split) / 100 / 100;

    $rows[$i][] = $row->private_notes;
    $rows[$i][] = $row->public_notes;

    $i++;
  }

  $th = array ( 'Date',
		'Time',
		'Tutor',
		'Student(s)',
		'Subject',
		'Duration (Original)',
		'Duration (Final)',
		'Rate (Original)',
		'Rate (Final)',
		'Total Student Cost',
		'Tutor Split',
		'Amount Due Tutor',
		'Private Notes',
		'Public Notes'
		);

  $headline = ($tutor_flag ? "Tutor: " : "Student: ")
    . theme('eto_user', $target_user)
    . " from " . date_format($start, "m/d/Y")
    . " to " . date_format($end, "m/d/Y")
    . " (report timezone: " . date_format($start, "T") . ")";
  if ($tutor_flag == -1) {
    $user_string = 'all';
  } else {
    $user_string = ($target_user->eto_profile->first_name ? $target_user->eto_profile->first_name . "-" . $target_user->eto_profile->last_name : $target_user->name);
  }
  preg_replace("/\s+/", "-", $user_string);
  $filename = "report_" . ($tutor_flag ? "tutor_" : "student_") . $user_string . "_" . date_format($start, "Y-m-d") . "_" . date_format($end, "Y-m-d") . ".csv";
  _write_file($filename, $headline, $th, $rows);
}

function _write_file($filename, $headline, $th, $rows) {
  $dir = file_directory_path() . '/reports';
  file_check_directory($dir, FILE_CREATE_DIRECTORY);
  $path = $dir . "/" . $filename;

  $fh = fopen($path,"w");

  $s = sputcsv( array($headline), ",", '"', "\r\n");
  fwrite($fh, $s);

  $s = sputcsv( $th, ",", '"', "\r\n");
  fwrite($fh, $s);

  foreach ($rows as $row) {
    $s = sputcsv( $row, ",", '"', "\r\n");
    fwrite($fh, $s);
  }
  fclose($fh);

  $details = stat($path);

  global $user;

  $file_obj->filename = $file;
  $file_obj->path = $dir;
  $file_obj->filemime = 'text/csv';
  $file_obj->filesize = $details['size'];
  $file_obj->filesource = $file;
  $file_obj->uid = 1;
  $file_obj->status = FILE_STATUS_TEMPORARY;
  $file_obj->timestamp = $time;
  $file_obj->list = 1;
  $file_obj->new = TRUE;

  drupal_write_record('files', $file_obj);

  global $user;
  watchdog('reports', t('Serving the Report: @headline to @name', array("@headline" => $headline,
									"@name" => $user->name)));

  drupal_goto($path);
}


// http://www.php.net/manual/en/function.fputcsv.php#96937

function sputcsv($row, $delimiter = ',', $enclosure = '"', $eol = "\n")
{
  static $fp = false;
  if ($fp === false)
    {
      $fp = fopen('php://temp', 'r+'); // see http://php.net/manual/en/wrappers.php.php - yes there are 2 '.php's on the end.
      // NB: anything you read/write to/from 'php://temp' is specific to this filehandle
    }
    else
      {
        rewind($fp);
      }

  if (fputcsv($fp, $row, $delimiter, $enclosure) === false)
    {
      return false;
    }

  rewind($fp);
  $csv = fgets($fp);

  if ($eol != PHP_EOL)
    {
      $csv = substr($csv, 0, (0 - strlen(PHP_EOL))) . $eol;
    }

  return $csv;
}




/************************************************************************
 ************************************************************************
 **
 ** Logged sessions
 **
 ************************************************************************
 ************************************************************************/

function eto_logged_sessions() {
  if ($_POST) {
    return process_logged_sessions($_POST);
  }
  // else
  $output = '';

  $sessions = eto_fetch_logged_sessions();

  $rows = array();
  foreach ($sessions as $row) {
    $date = date_make_date($row->start_datetime_utc, "UTC");
    date_timezone_set($date, timezone_open(user_tz($user)));
    $rows[$i][] = date_format($date, "m/d/Y g:ia");
    $tutor = get_session_log_tutor($row->id);
    $tutor = eto_user_load($tutor->uid);
    $rows[$i][] = theme("eto_user", $tutor);

    $students = get_session_log_students($row->id);
    $s_array = array();
    foreach ($students as $s) {
      $student = eto_user_load($s->uid);
      $s_array[] = theme("eto_user", $student);
    }
    $rows[$i][] = implode(", ", $s_array);

    $subject_term = taxonomy_get_term($row->subject_tid);
    if ($subject_term) {
      $subject = $subject_term->name;
    } else {
      $subject = "Unknown";
    }
    $rows[$i][] = $subject;
    $rows[$i][] = $row->duration_final / 60;
    $rows[$i][] = "<input type='checkbox' name='log-$row->id' value='log-$row->id' />";
    $i++;
  }

  $th = array( 'Date', 'Tutor', 'Student(s)', 'Subject', 'Duration', 'Logged?' );


  $output .= '<H2>Logged Sessions</h2>';
  $output .= '<p>The sessions below have not been entered into the accounting system.  After you have entered them, check the checkbox then click "Mark as Logged" and they will be removed from the list.</p>';
  $output .= '<form action="/eto/admin/logged-sessions"  accept-charset="UTF-8" method="post" id="sb-logged-sessions">';
  $output .= '<input type="submit" name="op" id="edit-submit-top" value="Mark as Logged"  class="form-submit" />';
  $output .= theme('table', $th, $rows);
  $output .= '<input type="submit" name="op" id="edit-submit-bottom" value="Mark as Logged"  class="form-submit" />';
  $output .= '</form>';
  return $output;
}


function process_logged_sessions($args) {

  $count = 0;
  foreach ($args as $arg) {
    list($foo, $id) = explode('-', $arg);
    if ($id) {
      $log = sb_session_log_load($id);
      if ($log) {
	$count ++;
	db_query("UPDATE {eto_session_log} SET accounting_flag = 1 WHERE id = %d",
		 $id);
      }
    }
  }
  drupal_set_message("$count logs updated");
  drupal_goto('/eto/admin/logged-sessions');
}
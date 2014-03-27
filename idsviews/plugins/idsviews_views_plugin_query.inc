<?php
// $Id;

/**
 * @file
 *   Views query plugin for the IDS API.
 */

class idsviews_views_plugin_query extends views_plugin_query_default {
  var $ids_wrapper;
  var $orderby;

  /**
   * Constructor; Create the basic query object and fill with default values.
   */
  function init($base_table, $base_field, $options) {
    parent::init($base_table, $base_field, $options);
    $this->ids_wrapper = new IdsApiWrapper();
  }

  function add_dataset($dataset) {
    if (isset($dataset)) {
      $this->ids_wrapper->setSite($dataset);
    }
  }

  function add_filter($group, $filter, $value) {
    if ($this->check_filter($group, $filter)) {
      $value = preg_replace('/\s*(\&|AND)\s*/i','&',$value); 
      $value = preg_replace('/\s*(\||OR)\s*/i','|',$value);
      $this->where[$group]['conditions'][] = array(
            'field' => $filter,
            'value' => $value
          );
    }
    else {
      watchdog('idsviews', 'Filter !filter cannot be added to more than one group. Filter ignored.', array('!filter' => $filter), WATCHDOG_NOTICE);
    }
  }

  function check_filter($group, $filter) {
    $found = FALSE;
    $groups = array_keys($this->where);
    while (($group_key = current($groups)) && (!$found)) {
      if (($group_key !== $group) && (!empty($this->where[$group_key]['conditions']))) {
        while ((list($field_key, $field_array) = each($this->where[$group_key]['conditions'])) && (!$found)) {
          $found = ($field_array['field'] === $filter);
        }
      }
      next($groups);
    }
    return (!$found);
  }

  function add_field($table, $field, $alias = '', $params = array()) {
    $field_info = array(
      'field' => $field,
      'table' => $table,
      'alias' => $field,
    ) + $params;
    if (empty($this->fields[$field])) {
      $this->fields[$field] = $field_info;
    }
    return $field;
  }

  function add_relationship($left_table, $left_field, $right_table, $right_field) {
      $this->add_field($left_table, $left_field);
      $this->joins[$left_table][$left_field] = array(
        'table' => $right_table,
        'field' => $right_field,
      );
    return FALSE;
  }

  public function add_orderby($table, $field = NULL, $order = "ASC", $alias = '', $params = array()){
    $this->orderby = array(
      'order' => $order,
      'orderby_field' => $field,
    );
  }

  public function build(&$view) {
    $view->init_pager();
  }

  /**
   * Executes the query and fills the associated view object with according
   * values.
   * @param view $view
   *   The view which is executed.
   */
  function execute(&$view) {
    $type = str_replace("idsviews_", "", $view->base_table);
    $this->ids_wrapper->setApiKey(variable_get('idsapi_api_key', ''));
    $this->ids_wrapper->setNumRequested($this->pager->options['items_per_page']);
    $this->ids_wrapper->setOffset($this->pager->current_page * $this->pager->options['items_per_page']);
    $this->ids_wrapper->setObjectType($type);
    $this->ids_wrapper->setFormat('full');
    $this->set_url();
    $start = microtime();
    $response = $this->ids_wrapper->makeRequest();
    $view->execute_time = microtime() - $start;
    $view->result = $response->getResults();
    $this->pager->total_items = $view->total_rows = $response->getTotalResults();
    $this->pager->update_page_info();
  }

  function set_url() {
    if (isset($this->where)) {
      foreach ($this->where as $group) {
        $params = array();
        if (isset($group['type'])) {
          if (strcasecmp($group['type'],"AND") == 0) {
            $oper = '&';
          }
          elseif (strcasecmp($group['type'],"OR") == 0) {
            $oper = '|';
          }
        }
        foreach ($group['conditions'] as $condition) {
          $field = $condition['field'];
          $value = $condition['value'];
          $field = $this->ids_parameter($field);
          $params[$field][] = $value;
        }
        foreach ($params as $param => $values) {
          $value = implode($oper, $values);
          if ($param === 'object_id') {
            $this->ids_wrapper->setObjectId($value);
            $this->ids_wrapper->setTypeRequest('get');
          }
          else {
            $this->ids_wrapper->setParam($param, $value);
          }
        }
      }
    }
    if (isset($this->orderby['order']) && (isset($this->orderby['orderby_field']))) {
      $order = 'sort_' . strtolower($this->orderby['order']);
      $this->ids_wrapper->setParam($order, $this->orderby['orderby_field']);
    }
    $dataset = variable_get('idsapi_default_dataset', IDS_API_DEFAULT_DATASET);
    $this->ids_wrapper->setSite($dataset);
  }

  function ids_parameter($field) {
    if ($field === 'query_filter') {
      $field = 'q';
    }
    else {
      $field = str_replace('_filter','',$field);
    }
    return $field;
  }

}

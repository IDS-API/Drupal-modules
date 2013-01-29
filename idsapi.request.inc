<?php

/**
 * IDS API Requests.
 */

// TODO Add interface, etc.

/**
 * Class IdsApiRequest.
 *
 */
// TODO Implement subclasses - Some properties and methods only make sense to specific types of requests (get, search, etc).
class IdsApiRequest {

  // API response, populated by IdsApiRequest->request()
  public $response;

  // The number of results to return, as requested.
  // Renamed in order to avoid confusion with the page size (the num_results parameter of the API).
  public $num_requested = 0;

  // The size of each page requested. Is equivalent to the API 'num_results' field.
  public $page_size = 0;

  // The index of the result to start from. If not specified then a value of 0 is used.
  public $start_offset;

  // The age of results to return (not older than the number of days specified by this attribute).
  // If not specified it is ignored.
  public $age_results;

  // Flag to designate if the response is from a cache.
  public $cache;

  // From where to get the information: eldis, bridge. Default is the defined default (eldis).
  public $site;

  // Type of request.
  public $type_request = 'search';

  // Identifier of the object being requested.
  public $object_id;

  // Type of object(s) requested: assets, documents, organisations, themes, subjects, regions, countries
  public $object_type;

  // Format of the response: id, short, full. If not specified, the defined default (short) is used.
  public $format = IDS_DEFAULT_FORMAT;

  // Parameters to be added to the request. Use IdsApiRequest->setSearchParam() to set.
  protected $search_params = array();

  // The cache table to use.
  protected $cache_table = IDS_CACHE; 

  // The URL of the request.
  public $url;

  /**
   * Constructor.
   * The search parameters are added later, calling to IdsApiRequest->setSearchParam() (for 'search' requests).
   */
  // TODO Implement subclases and factory method.
  function __construct() {
  }

  /**
   * Adds a search parameter.
   *
   */
  public function setSearchParam($key, $value) {
    $this->search_params[$key] = $value;
  }

  /**
   * Generates a query string based on the encoded search parameters.
   *
   */
  protected function queryString() {
    $query = '';
    if (!empty($this->search_params)) {
      $params = array();
      foreach ($this->search_params as $k => $v) {
        $params[] = $k . '=' . rawurlencode($v);
      }
      $query = implode('&', $params);
    }
    return $query;
  }

  /**
   * Builds the request path based on the object parameters.
   * Path: {site}/{type of request}/{object type}/[object id/]{format}
   */
  protected function setPath() {
    if (!isset($this->site)) {
      $this->site = variable_get('idsapi_default_dataset', IDS_DEFAULT_DATASET);
    }
    $path = $this->site;
    if (isset($this->type_request)) {
      $path .= '/' . $this->type_request;
    }  
    if (isset($this->object_type)) {
      $path .= '/' . $this->object_type;
    } 
    if (isset($this->object_id)) {
      $path .= '/' . $this->object_id;
    } 
    if (isset($this->format)) {
      $path .= '/' . $this->format;
    } 
    return $path;
  }

  /**
   * Builds the request URL, including the key and encoding the query string.
   * URL: {base url}/{site}/{type of request}/{object type}/[object id/]{format}
   */
  protected function setUrl() {
    $query_string = $this->queryString();
    $this->url = variable_get('idsapi_api_url', IDS_API_URL);
    $api_key = variable_get('idsapi_api_key', '');
    $api_key_par = IDS_API_KEY_PAR;
    $this->url .= $this->setPath() . '?' . $api_key_par . '=' . $api_key;
    if (($this->type_request === 'search') || ($this->type_request === 'get_all')) { 
      if ($this->age_results) {
        $days = $this->age_results . ' days';
        $today = date_create();
        $date = date_sub($today, date_interval_create_from_date_string($days));
        $metadata_published_after = date_format($date, 'Y-m-d');
        $this->url .= '&' . 'metadata_published_after=' . $metadata_published_after;
      }
      if ($query_string) {
        $this->url .= '&' . $query_string;
      }
      $this->setPageSize();
      if ($this->page_size) {
        $this->url .= '&' . 'num_results=' . $this->page_size;
      }
      if ($this->start_offset) {
        $this->url .= '&' . 'start_offset=' . $this->start_offset;
      }
    }
  }

  /**
   * Sets the num_results parameter in the URL to call the API.
   */
  protected function setPageSize() {
    // Values of 0 for num_requested are interpreted as 'all items' (when importing categories, in particular).
    // Search functions, fetchers, etc. should set num_requested if less results are expected.
    // Depending on the context, global settings 'idsapi_number_items' (IDS_NUM_ITEMS by default) or
    // 'idsapi_feeds_number_assets' (IDS_FEEDS_NUM_ASSETS by default) should be used.
    if (($this->type_request === 'search') || ($this->type_request === 'get_all')) {
      if ($this->num_requested) {
          if ($this->num_requested <= IDS_MAX_NUM_ITEMS) {
            $this->page_size = $this->num_requested;
          }
          else {
            // The page size is the max. allowed by the API.
            $this->page_size = IDS_MAX_NUM_ITEMS;
          }
      }
      else {
        // The number of results is not limited and the page size is the max. allowed by the API.
        $this->page_size = IDS_MAX_NUM_ITEMS;
      }
    }
  }

    /**
   * Builds the URL, checks if the request is in the cache, and if not makes the call and saves it in the cache.
   *
   * Format of the URL calls (not using 'friendly names' for the time being):
   * Get:     {base_url}/{site}/get/{object type}/{object id}/{format}?{key}
   * Get all: {base_url}/{site}/get_all/{object_type}/{format}?{key}
   * Search:  {base_url}/{site}/search/{object type}/{format}?{key}&{query}
   * Count: {base_url}/{site}/count/{object type}?{key}&{query}
   *
   * @return an IdsApiResponse object or FALSE if error.
   */
  public function makeRequest() {
    // Builds the URL.
    $this->setUrl();
    // Check if it's in the cache.  If it's not, perform the call and save it in the cache.
    if (0 || $cache = $this->cacheGet()) {
      list($results, $total_results) = $cache->data;
      idsapi_log($this->url, -1);
    }
    else {
      $ts = microtime(true);
      list($results, $total_results) = $this->getDecodedResults($this->url, 0);
      $t = microtime(true) - $ts;
      $this->cacheSet(array($results, $total_results));
      idsapi_log($this->url, $t);
    }
    // Create an IdsApiResponse and return it.
    if (!$results) {
      $results = array();
      $total_results = 0;
    }
    elseif (!isset($results[0])) {
      $results = array($results);
    }
    $response = new IdsApiResponse($results, $this->format, $this->object_type, $total_results, $this->site);
    return $response;
  }
  
  /**
   * Makes the actual API calls, implementing pagination and returning the decoded data.
   */
  protected function getDecodedResults($url, $num_retrieved) {
    $response = drupal_http_request($url);
    if ($response->code == '200') {
      $data = json_decode($response->data, true);
      if (is_array($data)) {
        $results = $data['results'];
        if (array_key_exists('metadata', $data)) {
          $metadata = $data['metadata'];
          $total_results = $metadata['total_results'];
          $num_results = count($results);
          $num_retrieved += $num_results;
          if (($total_results > $num_retrieved) && (($num_retrieved < $this->num_requested) || ($this->num_requested == 0))) {
            if (isset($metadata['next_page'])) {
              $next_page_url = $metadata['next_page'];
              if (list($new_results) = $this->getDecodedResults($next_page_url, $num_retrieved)) {
                $results = array_merge_recursive($results, $new_results);
              }
            }
          }
        }
        elseif (isset($results)) {
          $total_results = 1;
        }
        return array($results, $total_results);
      }
      else {
        watchdog('idsapi', 'Invalid API response received.', array('IdsApiRequest::getDecodedResults()'), WATCHDOG_ERROR);
      }
    }     
    else {
      watchdog('idsapi', 'HTTP error !code received.', array('!code' => $response->code), WATCHDOG_ERROR);
    }
	return FALSE;
  }
  
  /**
   * Retrieve the cache.  Wrapper around Drupal's cache_get().
   */
  protected function cacheGet($reset = FALSE) {
    static $items = array();
    $cid = $this->cacheId();
    if (!isset($items[$cid]) || $reset) {
      if ($items[$cid] = cache_get($cid, $this->cache_table)) {
        // Don't return temporary items more that 5 minutes old.
        if ($items[$cid]->expire === CACHE_TEMPORARY && $items[$cid]->created > (time() + 300)) {
          return FALSE;
        }
      }
      else {
        return FALSE;
      }
    }
    return $items[$cid];
  }

  /**
   * Populate the cache.  Wrapper around Drupal's cache_set().
   */
  protected function cacheSet($data) {
    if ($data === FALSE) {
      // If we don't get a valid response we set a temporary cache to avoid making repeated failed requests in a short period of time.
      cache_set($this->cacheId(), FALSE, $this->cache_table, CACHE_TEMPORARY);
    }
    else {
      $ttl = (int)variable_get('idsapi_cache_time', '');
      $expire = time() + $ttl;
      cache_set($this->cacheId(), $data, $this->cache_table, $expire);
    }
  }

  /**
   * Helper function to generate a cache id based on the class name and
   * hash of the url
   */
  protected function cacheId() {
    return get_class($this) .':'. md5($this->url);
  }

}

/* ---------------------- Additional related functions ------------------- */

/**
 * Log API hits.
 */
function idsapi_log($url = '', $time = '') {

  //watchdog('idsapi', $url, array(), $severity = WATCHDOG_NOTICE, NULL);
  static $log;

  if (!empty($url)) {
    if (!isset($log[$url])) {
      $log[$url] = array(
        'count' => 1,
        'url' => $url,
        'time' => $time,
      );
    }
    else {
      $log[$url]['count']++;
    }
  }
  else {
    return empty($log) ? array() : $log;
  }
}



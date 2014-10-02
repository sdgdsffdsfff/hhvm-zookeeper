<?hh

<<__NativeData("ZookeeperData")>>
class Zookeeper {

    /* class constants */
    const PERM_READ = 1;
    const PERM_WRITE = 2;
    const PERM_CREATE = 4;
    const PERM_DELETE = 8;
    const PERM_ADMIN = 16;
    const PERM_ALL = 31;
    const EPHEMERAL = 1;
    const SEQUENCE = 2;
    const EXPIRED_SESSION_STATE = -112;
    const AUTH_FAILED_STATE = -113;
    const CONNECTING_STATE = 1;
    const ASSOCIATING_STATE = 2;
    const CONNECTED_STATE = 3;
    const NOTCONNECTED_STATE = 999;
    const CREATED_EVENT = 1;
    const DELETED_EVENT = 2;
    const CHANGED_EVENT = 3;
    const CHILD_EVENT = 4;
    const SESSION_EVENT = -1;
    const NOTWATCHING_EVENT = -2;
    const LOG_LEVEL_ERROR = 1;
    const LOG_LEVEL_WARN = 2;
    const LOG_LEVEL_INFO = 3;
    const LOG_LEVEL_DEBUG = 4;
    const SYSTEMERROR = -1;
    const RUNTIMEINCONSISTENCY = -2;
    const DATAINCONSISTENCY = -3;
    const CONNECTIONLOSS = -4;
    const MARSHALLINGERROR = -5;
    const UNIMPLEMENTED = -6;
    const OPERATIONTIMEOUT = -7;
    const BADARGUMENTS = -8;
    const INVALIDSTATE = -9;
    const OK = 0;
    const APIERROR = -100;
    const NONODE = -101;
    const NOAUTH = -102;
    const BADVERSION = -103;
    const NOCHILDRENFOREPHEMERALS = -108;
    const NODEEXISTS = -110;
    const NOTEMPTY = -111;
    const SESSIONEXPIRED = -112;
    const INVALIDCALLBACK = -113;
    const INVALIDACL = -114;
    const AUTHFAILED = -115;
    const CLOSING = -116;
    const NOTHING = -117;
    const SESSIONMOVED = -118;

    <<__Native>>
    public function __construct(string $host = '',mixed $watcher_cb = null, int $recv_timeout = 10000): void;

    <<__Native>>
    public function connect(string $host,mixed $watcher_cb = null, int $recv_timeout = 10000): bool;

    <<__Native>>
    public function create(string $path, string $value, mixed $acls, int $flags = 0 ): bool;

    <<__Native>>
    public function get(string $path , mixed  $watcher_cb = null, mixed &$stat_info = null,int $max_size = 0): mixed;

    <<__Native>>
    public function set(string $path, mixed $data, int $version = -1, mixed &$stat_info = null ): bool;

    <<__Native>>
    public function exists(string $path, mixed $watcher_cb= null ) : bool;

    <<__Native>>
    public function delete(string $path,int $version = -1 ): bool;

    <<__Native>>
    public function getClientId( ) : mixed;
    
    <<__Native>>
    public function getState( ) : int;

    <<__Native>>
    public function getRecvTimeout( ): int;
    
    <<__Native>>
    public function isRecoverable( ) : bool;

    <<__Native>>
    static public function setDebugLevel(int $level ): bool;
    
    <<__Native>>
    public function addAuth(string  $scheme, string $cert,  mixed  $completion_cb = null ): bool;
    
    <<__Native>>
   public function getAcl(string $path ): mixed;
    
    <<__Native>>
    public function setAcl(string $path, int $version,  mixed  $acls ): bool;

    <<__Native>>
    public function getChildren(string $path, mixed $watcher_cb= null ) : bool;

    <<__Native>>
    static public function setDeterministicConnOrder(bool $trueOrFalse ): bool;
    /*
     <<__Native>>
     public function setWatcher( mixed $watcher_cb): bool;

     <<__Native>>
     public function setLogFile(string $file ) : mixed;

     <<__Native>>
     public function getResultMessage( ) : mixed;
*/
}


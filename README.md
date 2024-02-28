# victo-rdb
Victo: Realtime AI Native Database

# Introduction
Welcome to Victo project.

Victo is a realtime AI Native Database for effeciently storing and retriving vectors. Victo is a database server leveraging the power of websockets which means to connect to Victo all that we need is a websocket client. Connect seamlessly with application written in any language, unlocking the potential for realtime communication and efficient vector operations.

We hope you enjoy using Victo.

<a name="readme-top"></a>

<details open="open">
  <summary>Table of Contents</summary>
  <ol>
    <li>
        <a href="#get-started">Get Started</a>
        <ul>
            <li><a href="#build">Build</a></li>
            <li><a href="#docker-support">Docker Support</a></li>
        </ul>
    </li>
    <li><a href="#technologies-and-concepts">Technologies and Concepts</a></li>
    <li><a href="#how-it-works">How it works?</a></li>
    <li><a href="#authentication">Authentication</a></li>
    <li>
        <a href="#db-operations-and-syntax">DB Operations and Syntax</a>
        <ul>
            <li><a href="#add-a-database">Add a database</a></li>
            <li><a href="#add-User">Add User</a></li>
            <li><a href="#change-password-for-self">Change password for self</a></li>
            <li><a href="#change-password-for-another-user-as-admin">Change password for another User as Admin</a></li>
            <li><a href="#add-a-collection">Add a collection</a></li>
            <li><a href="#delete-a-collection">Delete a collection</a></li>
            <li><a href="#list-total-number-of-collections-in-a-database">List total number of collections in a database</a></li>
            <li><a href="#list-all-collections-in-a-database">List all collections in a database</a></li>
            <li><a href="#add-a-vector-to-a-collection">Add a vector to a collection</a></li>
            <li><a href="#retrive-a-vector-from-a-collection">Retrive a vector from a collection</a></li>
            <li><a href="#delete-a-vector-in-a-collection">Delete a vector in a collection</a></li>
            <li><a href="#list-total-number-of-vectors-in-a-collection">List total number of vectors in a collection</a></li> 
            <li><a href="#list-all-vectors-in-a-collection">List all vectors in a collection</a></li>
            <li><a href="#query-a-vector">Query a vector</a></li> 
            <li><a href="#add-a-subscription">Add a Subscription</a></li> 
            <li><a href="#retrive-a-subscription-from-a-collection">Retrive a subscription from a collection</a></li> 
            <li><a href="#delete-a-subscription-in-a-collection">Delete a subscription in a collection</a></li>
            <li><a href="#list-total-number-of-subscriptions-in-a-collection">List total number of subscriptions in a collection</a></li> 
            <li><a href="#list-all-subscriptions-in-a-collection">List all subscriptions in a collection</a></li>
        </ul>
    </li>
    <li><a href="#logs-and-monitoring">Logs and Monitoring</a></li>
    <li><a href="#practical-usecases">Practical Usecases</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#version">Version</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#acknowledgements">Acknowledgements</a></li>
  </ol>
</details>

# Get Started

<div align="center"><h4>The current version of Victo works only in Unix and Unix-like OS</h4></div>

## Prerequisites
1. GCC
2. CMake (VERSION 3.27.1)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Build

1. Fork the repo
2. Clone your forked repo
3. CMake is used to manage the build process. In the cloned repo, navigate to the root directory
```
cd path/to/cloned/repo
```
Replace "path/to/cloned/repo" with actual path to your cloned repo
4. Build the executable using following commands
```
cmake -S . -B ./build
make -C ./build/
```
5. Once the step 4 is successful, "victo-exe" would have been created in "path/to/cloned/repo/<b>build</b>" directory
6. Navigate to "build" directory and start Victo database using below command
```
./build/victo-exe -d /preferred/path/for/victo/data/files
```
It is in this path the actual victo database objects and files are stored in the disk. Other supported flags including -d are:

| Flag     | Description                                          | Mandatory?        | Default Value                                      | Valid values  |
|----------|------------------------------------------------------|-------------------|----------------------------------------------------|---------------|
|   -d    | Preferred path to victo database                      | Yes               |                                                    | File Path     |
|   -i    | overwrite ip (useful while starting in container)     | No                | 127.0.0.1 or 0.0.0.0                               | IP Address    |
|         |                                                       |                   | 127.0.0.1 - localhost                              |               |
|         |                                                       |                   | 0.0.0.0   - for routing request outside container  |               | 
|   -p    | overwrite port                                        | No                | 2018                                               | Port Number   |
|   -l    | Logger Level                                          | No                | INFO                                               | DEBUG         |
|         |                                                       |                   |                                                    | INFO          |
|         |                                                       |                   |                                                    | WARN          |
|         |                                                       |                   |                                                    | ERROR         |
|         |                                                       |                   |                                                    | CRITICAL      |
|   -a    | Enable authentication and authorization               | No                | false                                              | true or false | 
|   -s    | Enable subscription                                   | No                | true                                               | true or false |      


7. By default, the db server is started listening on port 2018. We can connect to the victo sb server with any websocket client using the URL as below
```
ws://<host-ip>:2018/
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Docker Support

Alternatively, you can run Victo as a Docker container. 

### How to start Victo as a Docker Container?

Starting a Victo Server Instance as a Docker container is simple
```
docker run --name some-name -p <host-port>:8080 -v <host-path>:/app/victodb  -d sreehari006/victo:tag
```
... where "some-name" is the name you want to provide to the container. Inside the container Victo runs on port 8080. <host-port> is the port on host machine that you want to map to the container port. <host-path> is the path on host machine where the volume data is stored. "tag" is the version of Victo that you want to run.

The default ENTRPOINT for Victo docker image is 
```
ENTRYPOINT ["/app/exe/victo-exe", "-d", "/app/victodb", "-i", "0.0.0.0", "-p", "8080"]
```
However, you cna overwrite the ENTRYPOINT based on the details summarized under "Build" section above, point 6.

For more detail, please visit [https://hub.docker.com/repository/docker/sreehari006/victo/general](https://hub.docker.com/repository/docker/sreehari006/victo/general)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

# Technologies and Concepts
1. AI and Vector analysis
2. WebSockets and Socket programming
3. JSON parser
4. Abstract Syntax Tree
5. UUID generator

<p align="right">(<a href="#readme-top">back to top</a>)</p>

# How it works?
The fundamental components of Victo Database are:
* <b>db: </b>The basic components which holds a list of collections. Victo databse server can have multiple database objects.
* <b>collection: </b>Collection holds a list of vectors
* <b>vector: </b>Vectors are data points or entities which represent vector embeddings.
* <b>queries: </b>Queries are JSON-like string used to interact with database for storing and retriving db objects. 
* <b>subscription: </b>Subscription are like queries. It works in a event driven fashion. when there is a new vector added mathcing the criteria of any subscription, the client would be instantly notified.
* <b>User: </b>User is db user. Valid only if authentication control is enabled on server.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Authentication

The connection request should have a valid "Authorization Bearer Token" included in the Websocket HTTP(s) request header for authentication.

```
Authorization: Basic <base64-encoded-token>
```
"base64-encoded-token" here is base64 encoded string of "username:password". Kindly note, colon (:) is the delimiter between username and password.

Sample Header:
```
GET ws://127.0.0.1:2018/ HTTP/1.1
Host: 127.0.0.1:2018
Connection: Upgrade
Pragma: no-cache
Cache-Control: no-cache
Upgrade: websocket
Sec-WebSocket-Version: 13
Accept-Encoding: gzip, deflate, br
Accept-Language: en-GB,en-US;q=0.9,en;q=0.8
Sec-WebSocket-Key: <Sec-WebSocket-Key>
Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits
Authorization: Basic <base64-encoded-token>
```

When the server is started for the first time, a default "admin" user with a random password would be created. The password for admin user can be found in the logs while server startup. Please change the password immediately for security reasons.

With "admin" account, the additional user accounts can be created using <a href="#add-user">Add User</a> query.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## DB Operations and Syntax
The following list of DB operations can be executed on the server using any websocket client

### Basic structure of a query
As mentioned earlier, query is a JSON-like string. Any victo query has 3 basic components

| Key     | Description                                           | Valid values                                                               |
|---------|-------------------------------------------------------|----------------------------------------------------------------------------|
| op      | operation on the Database                             | "add" if the obj is db                                                     |
|         |                                                       | "add", "delete", "list" or "count" if obj is collection                    |
|         |                                                       | "put", "get", "list", "count", "delete" or "query" if obj is vector        |
|         |                                                       | "add", "get", "list", "count" or "delete" if obj is subscription           |
|         |                                                       | "add", "ch_pass", "ch_my_pass" if obj is user                              |
| obj     | Database object on which the operation is executed    | "db", "collection", "vector", "subscription" or "user"                     |
| args    | arguments                                             |                                                                            |

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Add a database 

#### Request
```
{   
    "op":"add", 
    "obj":"db", 
    "args": {
        "db": <preferred-name>
    }
}
```

| Key     | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|---------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db      | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Add User 

#### Request
```
{   
    "op":"add", 
    "obj":"user", 
    "args": {
        "name": <preferred-name>,
        "password": <preferred-password>
    }
}
```

| Key      | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|----------|-----------------------|-------------------|---------------|-------------------------------------------------|
| name     | User Name             | Yes               | String        | Any                                             |
| password | Password              | Yes               | String        | Any                                             |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Change password for self 

#### Request
```
{   
    "op":"ch_my_pass", 
    "obj":"user", 
    "args": {
        "name": <name>,
        "password": <password>,
        "new_password": <preferred-new-password>
    }
}
```

| Key          | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|--------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| name         | User Name             | Yes               | String        | Any                                             |
| password     | Password              | Yes               | String        | Any                                             |
| new_password | Password              | Yes               | String        | Any                                             |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Change password for another User as Admin

#### Request
```
{   
    "op":"ch_pass", 
    "obj":"user", 
    "args": {
        "name": <name>,
        "password": <password>,
        "new_password": <preferred-new-password>
    }
}
```

| Key          | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|--------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| name         | User Name             | Yes               | String        | Any                                             |
| password     | Password              | Yes               | String        | Any                                             |
| new_password | Password              | Yes               | String        | Any                                             |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Add a collection 

#### Request
```
{   
    "op":"add", 
    "obj":"collection", 
    "args": {
        "db": <db-name>,
        "collection": <preferred-name>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection  | Collection Name       | Yes               | String        | Alphanumeric value with hyphen and underscore   |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Delete a collection 

#### Request
```
{   
    "op":"delete", 
    "obj":"collection", 
    "args": {
        "db": <db-name>,
        "collection": <collection-name>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection  | Collection Name       | Yes               | String        | Alphanumeric value with hyphen and underscore   |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### List total number of collections in a database 

#### Request
```
{   
    "op":"count", 
    "obj":"collection", 
    "args": {
        "db": <db-name>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>,
            "count": <integer>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### List all collections in a database 

#### Request
```
{   
    "op":"list", 
    "obj":"collection", 
    "args": {
        "db": <db-name>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>,
            "collections: [
                "array-of-collections-uuid"
            ]
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Add a vector to a collection 

#### Request
```
{   
    "op":"put", 
    "obj":"vector", 
    "args": {
        "db": <db-name>,
        "collection": <collection-name>,
        "ai_model": <ai-model-with-which-vector-embeddings-are-generated>,
        "vdim": <vector-dimension">,
        "vp": <array-of-vector-datapoints>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection  | Collection Name       | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| ai_model    | AI Model              | Yes               | String        |                                                 |
| vdim        | Vector Dimension      | Yes               | Integer       |                                                 |
| vp          | Vector Points         | Yes               | Decimal Array |                                                 |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>,
            "hash": <uuid>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Retrive a vector from a collection 

#### Request
```
{   
    "op":"get", 
    "obj":"vector", 
    "args": {
        "db": <db-name>,
        "collection": <collection-name>,
        "hash": <vector-id>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection  | Collection Name       | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| hash        | Vector ID             | Yes               | UUID          |                                                 |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>,
            "hash": <uuid>,
            "ai_model": <ai-model>,
            "dimension": <integer>,
            "is_normal": <boolean-true-if-normalized-else-false>,
            "vp": <array-of-decimal-datapoints>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Delete a vector in a collection 

#### Request
```
{   
    "op":"get", 
    "obj":"vector", 
    "args": {
        "db": <db-name>,
        "collection": <collection-name>,
        "hash": <vector-id>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection  | Collection Name       | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| hash        | Vector ID             | Yes               | UUID          |                                                 |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### List total number of vectors in a collection 

#### Request
```
{   
    "op":"count", 
    "obj":"vector", 
    "args": {
        "db": <db-name>,
        "collection": <collection-name>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection  | Collection Name       | Yes               | String        | Alphanumeric value with hyphen and underscore   |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>,
            "count": <integer>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### List all vectors in a collection 

#### Request
```
{   
    "op":"list", 
    "obj":"vector", 
    "args": {
        "db": <db-name>,
        "collection": <collection-name>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection  | Collection Name       | Yes               | String        | Alphanumeric value with hyphen and underscore   |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>,
            "vectors": [
                <array-of-vector-uuids>
            ]
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Query a vector 

#### Request
```
{   
    "op":"query", 
    "obj":"vector", 
    "args": {
        "db": <db-name>,
        "collection": <collection-name>,
        "ai_model": <ai-model-with-which-vector-embeddings-are-generated>,
        "vdim": <vector-dimension">,
        "vp": <array-of-vector-datapoints>
        "q_ops": {
            "vd_method": <preferred-vector-distance-calculation-method>,
            "k_value": <base-value-for-finding-nearest-vectors>,
            "logical_op": <logic-operation-in-query-comparison-against-kvalue>,
            "limit": <number-of-objects-returned-in-query>,
            "order": <order-in-which-query-is-returned>,
            "include_fault": <include-fault-nodes>,
            "do_normal": <normalize-before-query-comparison>,
            "p_value": <applicable-only-for-minskowski-method>
        }
    }
}
```

| Key             | Description                                       | Is Mandatory?     | Data Type     | Valid values                                    |
|-----------------|-----------------------|-------------------|---------------|-----------------------------------------------------------------------------|
| db              | DB Name                                           | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection      | Collection Name                                   | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| ai_model        | AI Model                                          | Yes               | String        |                                                 |
| vdim            | Vector Dimension                                  | Yes               | Integer       |                                                 |
| vp              | Vector Points                                     | Yes               | Decimal Array |                                                 |
| vd_method       | Vector Distance Calculation Method                | No                | String        | Default value is EUCLIDEAN_DISTANC              |
|                                                                                                         | EUCLIDEAN_DISTANCE                              |
|                                                                                                         | COSINE_SIMILARITY                               |
|                                                                                                         | MANHATTAN_DISTANCE                              |
|                                                                                                         | MINKOWSKI_DISTANCE                              |
|                                                                                                         | DOT_PRODUCT                                     |
| k_value         | base value used for query on calculated distance  | No                | Decimal       | Default 0.0                                     |
| logical_op      | logical operation against k_value while query     | No                | String        | Default is =                                    |
|                                                                                                         | >=  - GREATER_THAN_OR_EQUAL_TO                  |
|                                                                                                         | >  - GREATER_THAN                               |
|                                                                                                         | =  - EQUAL_TO                                   |
|                                                                                                         | < - LESS_THAN                                   |
|                                                                                                         | <= - LESS_THAN_OR_EQUAL_TO                      |
| limit           | Number of vector points returned                  | No                | Integer       | Positive Integer                                |
| order           | Ascending or descending based on k_value          | No                | Boolean       | Default is false                                |
|                                                                                                         | false - Ascending                               |
|                                                                                                         | true - Descending                               |
| include_fault   | Include fault nodes                               | No                | Boolean       | Default is false                                |
|                                                                                                         | false - do not include fault nodes              |
|                                                                                                         | true - include fault nodes                      |
| do_normal       | Normalize vector points while query               | No                | Boolean       | Default is false                                |
|                                                                                                         | false - do not normalize                        |
|                                                                                                         | true - normalize vector                         |
| p_value         | Applicable only for Minskowski Method             | Conditional       | Decimal       |                                                 |

<i>Fault nodes are something where the query vector and vectors are incompatible. For example, if the query vector dimension is not same as vector data points dimension.</i>

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>,
            "vectors": [
                {
                    "code": <interger-result-code>,
                    "message": <result-message>,
                    "ai_model": <ai-model>,
                    "hash": <vector-uuid>,
                    "is_normal": <Y-if-normalized-else-N>,
                    "dimension": <integer-vector-dimension>,
                    "distance": <distance-from-k_value>
                }
            ],
            "faults": [
                {
                    "code": <interger-result-code>,
                    "message": <result-message>,
                    "ai_model": <ai-model>,
                    "hash": <vector-uuid>,
                    "is_normal": <Y-if-normalized-else-N>,
                    "dimension": <integer-vector-dimension>,
                    "distance": 0
                }

            ]
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Add a Subscription 

#### Request
```
{   
    "op":"add", 
    "obj":"subscription", 
    "args": {
        "db": <db-name>,
        "collection": <collection-name>,
        "ai_model": <ai-model-with-which-vector-embeddings-are-generated>,
        "vdim": <vector-dimension">,
        "vp": <array-of-vector-datapoints>,
        "is_normal": <normalize-vector-points-before-query>,
        "q_ops": {
            "vd_method": <preferred-vector-distance-calculation-method>,
            "k_value": <base-value-for-finding-nearest-vectors>,
            "logical_op": <logic-operation-in-query-comparison-against-kvalue>,
            "p_value": <applicable-only-for-minskowski-method>
        }
    }
}
```

| Key             | Description                                       | Is Mandatory?     | Data Type     | Valid values                                    |
|-----------------|-----------------------|-------------------|---------------|-----------------------------------------------------------------------------|
| db              | DB Name                                           | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection      | Collection Name                                   | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| ai_model        | AI Model                                          | Yes               | String        |                                                 |
| vdim            | Vector Dimension                                  | Yes               | Integer       |                                                 |
| vp              | Vector Points                                     | Yes               | Decimal Array |                                                 |
| vd_method       | Vector Distance Calculation Method                | No                | String        | Default value is EUCLIDEAN_DISTANCE             |
|                                                                                                         | EUCLIDEAN_DISTANCE                              |
|                                                                                                         | COSINE_SIMILARITY                               |
|                                                                                                         | MANHATTAN_DISTANCE                              |
|                                                                                                         | MINKOWSKI_DISTANCE                              |
|                                                                                                         | DOT_PRODUCT                                     |
| is_normal       | Normalize vector points before query              | No                | boolean       | true or false. Default: false                   |
| k_value         | base value used for query on calculated distance  | No                | Decimal       | Default 0.0                                     |
| logical_op      | logical operation against k_value while query     | No                | String        | Default is =                                    |
|                                                                                                         | >=  - GREATER_THAN_OR_EQUAL_TO                  |
|                                                                                                         | >  - GREATER_THAN                               |
|                                                                                                         | =  - EQUAL_TO                                   |
|                                                                                                         | < - LESS_THAN                                   |
|                                                                                                         | <= - LESS_THAN_OR_EQUAL_TO                      |
| p_value         | Applicable only for Minskowski Method             | Conditional       | Decimal       |                                                 |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>,
            "hash": <subscription-hash>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

### Retrive a subscription from a collection 

#### Request
```
{   
    "op":"get", 
    "obj":"subscription", 
    "args": {
        "db": <db-name>,
        "collection": <collection-name>,
        "hash": <subscription-id>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection  | Collection Name       | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| hash        | Subscription ID       | Yes               | UUID          |                                                 |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>,
            "hash": <uuid>,
            "client_id": <uuid>,
            "ai_model": <ai-model>,
            "dimension": <integer>,
            "is_normal": <boolean-true-if-normalized-else-false>,
            "vp": <array-of-decimal-datapoints>
            "q_ops": {
                "vd_method": <preferred-vector-distance-calculation-method>,
                "k_value": <base-value-for-finding-nearest-vectors>,
                "logical_op": <logic-operation-in-query-comparison-against-kvalue>,
                "p_value": <applicable-only-for-minskowski-method>             
            }
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Delete a subscription in a collection 

#### Request
```
{   
    "op":"get", 
    "obj":"vector", 
    "args": {
        "db": <db-name>,
        "collection": <collection-name>,
        "hash": <subscription-id>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection  | Collection Name       | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| hash        | Subscription ID       | Yes               | UUID          |                                                 |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### List total number of subscriptions in a collection 

#### Request
```
{   
    "op":"count", 
    "obj":"subscription", 
    "args": {
        "db": <db-name>,
        "collection": <collection-name>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection  | Collection Name       | Yes               | String        | Alphanumeric value with hyphen and underscore   |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>,
            "count": <integer>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### List all subscriptions in a collection 

#### Request
```
{   
    "op":"list", 
    "obj":"subscription", 
    "args": {
        "db": <db-name>,
        "collection": <collection-name>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection  | Collection Name       | Yes               | String        | Alphanumeric value with hyphen and underscore   |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>,
            "subscriptions": [
                <array-of-vector-uuids>
            ]
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Logs and Monitoring

By default, a log file (log.txt) would be generated under "database-base-path/logs/" location.

The database server can be started with following logger levels using -l flag

- DEBUG: Used for detailed debugging information. 
- INFO: Used to provide informational messages.
- WARNING: Used to indicate potential issues or anomalies that are not necessarily errors but might require attention.
- ERROR: Used to indicate errors that caused the server to behave unexpectedly or incorrectly, but that are not fatal.
- CRITICAL: Used to indicate critical errors that require immediate attention and might result in the termination of the server.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

# Practical Usecases
* NLP
* Generative AI
* Recommender System
* Viedo Search
* Image Search
* eCommerce
* Machine Learning
* Social Networks

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Roadmap
- [x] Enhance logging mechanism and Error Handling
- [x] Enhance DB features
    - [x] Users accounts and access control
    - [x] Subscribe on collections
- [ ] Build scheduler for threads handling db operations
- [ ] Enable support for secure websockets (wss)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Version
[15 Feb 2024] - 2.0.0 (Latest stable version)
Added support for:
- Authentication
- User accounts and access control
- Subscriptions

[26 Jan 2024] - 1.2.1
Enhanced logging mechanism performance improved

[20 Jan 2024] - 1.2.0
Enhanced logging mechanism and error handling

[12 Jan 2024] - 1.1.0
Added support for Docker and containerization

[6th Jan 2024] - 1.0.0 

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## License

Distributed under the MIT License. See `LICENSE.txt` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

# Contributing

Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right">(<a href="#readme-top">back to top</a>)</p>

# Contact

Sree Hari 
eMail - hari.tinyblitz@gmail.com
LinkedIn - [view_my_profile] (https://www.linkedin.com/in/sree-hari-84911b123/)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

# Acknowledgements

1. [Best-README-Template](https://github.com/othneildrew/Best-README-Template)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

-module (queue_ejercicio).
-export ([empty/0, insert/2, remove/1]).

empty() -> 
    {[], []}.

insert(Queue, Elem) ->
    {Front, Back} = Queue,
    {Front, [Elem | Back]}.

remove(Queue) ->
    case Queue of
        {[], []} ->
            empty;
        {[], Back} ->
            remove({lists:reverse(Back), []});
        {[X | Front], Back} ->
            {ok, X, {Front, Back}}
    end.


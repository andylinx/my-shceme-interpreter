## Extension for lazy_evaluation
You can open this optimization by define Lazy_tag

By changing the definition of Assoc, we can choose to not call the eval function the moment we encounter a varible.
Instead, we can evaluate them when they are needed.
To do so, we need to store the expression and the environment of such varibles!

And when we call the find function, we need to verify whether such varible has already been calculated.
If not, evaluate it during the find function and set the flag representing whether it has been calculated to true.

Additionally, for some expressions like car/cdr, we just need to calculate half part of them.
However, I could only deal with the cons / quote type, other type is hard to implement.
I'd appreciate it if you can commit some idea.
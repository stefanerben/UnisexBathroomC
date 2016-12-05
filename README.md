# UnisexBathroomC

##Aufgabenstellung##

I wrote this problem when a friend of mine left her position teaching physics at Colby College and took a job at 
Xerox. She was working in a cubicle in the basement of a concrete monolith, and the nearest women's bathroom was 
two floors up. She proposed to the Uberboss that they convert the men's bathroom on her floor to a unisex bathroom, 
sort of like on Ally McBeal. The Uberboss agreed, provided that the following synchronization constraints can be 
maintained:

    1. There cannot be men and women in the bathroom at the same time.
    2. There should never be more than three employees squandering company time in the bathroom.

Of course the solution should avoid deadlock. For now, though, don't worry about starvation. You may assume that 
the bathroom is equipped with all the semaphores you need. 
<script lang="ts">
	import { onMount } from 'svelte';
	import ArrowUpIcon from 'lucide-svelte/icons/arrow-up';

	let visible = $state(false);

	onMount(() => {
		const onScroll = () => {
			visible = window.scrollY > 300;
		};
		window.addEventListener('scroll', onScroll, { passive: true });
		return () => window.removeEventListener('scroll', onScroll);
	});

	function scrollToTop() {
		window.scrollTo({ top: 0, behavior: 'smooth' });
	}
</script>

{#if visible}
	<button
		onclick={scrollToTop}
		class="fixed right-6 bottom-6 z-50 flex h-10 w-10 cursor-pointer items-center justify-center rounded-full border border-border bg-card text-foreground shadow-md transition-all hover:border-primary/40 hover:bg-primary/10 hover:text-primary"
		aria-label="Scroll to top"
	>
		<ArrowUpIcon size={18} strokeWidth={1.5} />
	</button>
{/if}

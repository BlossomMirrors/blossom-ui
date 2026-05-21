<script lang="ts">
	import type { Snippet } from 'svelte';

	let { subheader, size = 'lg', class: cls = '', children }: {
		subheader: string;
		size?: 'lg' | 'sm';
		class?: string;
		children: Snippet;
	} = $props();
</script>

<div class={cls}>
	<p class="mb-3 text-xs font-semibold tracking-widest text-primary uppercase">{subheader}</p>
	<h2 class={size === 'lg' ? 'font-serif text-5xl leading-tight md:text-6xl' : 'font-serif text-4xl leading-tight md:text-5xl'}>
		{@render children()}
	</h2>
</div>
